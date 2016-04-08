#pragma once

#include "defs.h"
#include "nodes_and_edges.h"
#include "graph.h"
#include "chgraph.h"
#include "prioritizer.h"

#include <chrono>
#include <queue>
#include <mutex>
#include <vector>
#include <omp.h>
#include <algorithm>

namespace chc
{

namespace unit_tests
{
	void testCHConstructor();
}

namespace
{
	uint MAX_UINT(std::numeric_limits<uint>::max());
}

template <typename NodeT, typename EdgeT>
class CHConstructor{
	private:
		// typedef CHNode<NodeT> LvlNode;
		typedef CHEdge<EdgeT> Shortcut;
		typedef CHGraph<NodeT, EdgeT> CHGraphT;

		struct CompInOutProduct;
		struct PQElement;
		typedef std::priority_queue<
				PQElement, std::vector<PQElement>, std::greater<PQElement> > PQ;

		CHGraphT& _base_graph;

		struct ThreadData {
			PQ pq;
			std::vector<uint> dists;
			std::vector<uint> reset_dists;
		};
		std::vector<ThreadData> _thread_data;

		uint _num_threads;
		ThreadData& _myThreadData();

		std::vector<Shortcut> _new_shortcuts;
		std::vector<int> _edge_diffs;
		std::vector<NodeID> _remove;
		std::vector<bool> _to_remove;
		std::mutex _new_shortcuts_mutex;


		void _initVectors();
		void _contract(NodeID node);
		std::vector<Shortcut> _contract(NodeID node, ThreadData& td) const;
		void _quickContract(NodeID node);
		std::vector<Shortcut> _calcShortcuts(Shortcut const& start_edge, NodeID center_node,
				EdgeType direction, ThreadData& td) const;
		void _calcShortestDists(ThreadData& td, NodeID start_node, EdgeType direction,
				uint radius) const;
		Shortcut _createShortcut(Shortcut const& edge1, Shortcut const& edge2,
				EdgeType direction = EdgeType::OUT) const;

		void _markNeighbours(NodeID node, std::vector<bool>& marked) const;

		void _chooseRemoveNodes(std::vector<NodeID> const& independent_set);
		void _chooseAllForRemove(std::vector<NodeID> const& independent_set);
		void _removeNodes(std::vector<NodeID>& nodes);
	public:
		CHConstructor(CHGraphT& base_graph, uint num_threads = 1);

		/* functions for contraction */
		void quickContract(std::vector<NodeID>& nodes, uint max_degree,
				uint max_rounds);
		void contract(std::vector<NodeID>& nodes);
		void contract(std::vector<NodeID>& nodes, Prioritizer& prioritizer);
		void rebuildCompleteGraph();

		/* const functions that use algorithms from the CHConstructor */
		std::vector<NodeID> calcIndependentSet(std::vector<NodeID> const& nodes,
				uint max_degree = MAX_UINT) const;
		int calcEdgeDiff(NodeID node) const;
		std::vector<int> calcEdgeDiffs(std::vector<NodeID> const& nodes) const;
		std::vector<Shortcut> getShortcutsOfContracting(NodeID node) const;
		std::vector<std::vector<Shortcut>> getShortcutsOfContracting(std::vector<NodeID> const& nodes) const;
		std::vector<Shortcut> getShortcutsOfQuickContracting(NodeID node) const;
		std::vector<std::vector<Shortcut>> getShortcutsOfQuickContracting(std::vector<NodeID> const& nodes) const;

		friend void unit_tests::testCHConstructor();
};

/*
 * private
 */

template <typename NodeT, typename EdgeT>
struct CHConstructor<NodeT, EdgeT>::CompInOutProduct
{
	CHGraphT const& g;

	CompInOutProduct(CHGraphT const& g)
		: g(g) {}

	bool operator()(NodeID node1, NodeID node2) const
	{
		uint edge_product1(g.getNrOfEdges(node1, EdgeType::IN)
				* g.getNrOfEdges(node1, EdgeType::OUT));
		uint edge_product2(g.getNrOfEdges(node2, EdgeType::IN)
				* g.getNrOfEdges(node2, EdgeType::OUT));

		return edge_product1 < edge_product2;
	}
};

template <typename NodeT, typename EdgeT>
struct CHConstructor<NodeT, EdgeT>::PQElement
{
	NodeID node;
	uint _dist;

	PQElement(NodeID node, uint dist)
		: node(node), _dist(dist) {}

	bool operator>(PQElement const& other) const
	{
		return _dist > other._dist;
	}

	/* make interface look similar to an edge */
	uint distance() const { return _dist; }
};

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::_myThreadData() -> ThreadData&
{
	return _thread_data[omp_get_thread_num()];
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_initVectors()
{
	_new_shortcuts.clear();
	_remove.clear();
	_to_remove.assign(_base_graph.getNrOfNodes(), false);
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_contract(NodeID node)
{
	ThreadData& td(_myThreadData());
	auto shortcuts(_contract(node, td));

	_edge_diffs[node] = int(shortcuts.size()) - int(_base_graph.getNrOfEdges(node));

	std::unique_lock<std::mutex> lock(_new_shortcuts_mutex);
	_new_shortcuts.insert(_new_shortcuts.end(), shortcuts.begin(), shortcuts.end());
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::_contract(NodeID node, ThreadData& td) const -> std::vector<Shortcut>
{
	EdgeType search_direction;

	if (_base_graph.getNrOfEdges(node, EdgeType::IN) <= _base_graph.getNrOfEdges(node, EdgeType::OUT)) {
		search_direction = EdgeType::OUT;
	}
	else {
		search_direction = EdgeType::IN;
	}

	std::vector<Shortcut> shortcuts;
	for (auto const& edge: _base_graph.nodeEdges(node, !search_direction)) {
		if (edge.tgt == edge.src) continue; /* skip loops */
		auto new_shortcuts(_calcShortcuts(edge, node, search_direction, td));
		shortcuts.insert(shortcuts.end(), new_shortcuts.begin(), new_shortcuts.end());
	}

	return shortcuts;
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_quickContract(NodeID node)
{
	auto shortcuts(getShortcutsOfQuickContracting(node));

	std::unique_lock<std::mutex> lock(_new_shortcuts_mutex);
	_new_shortcuts.insert(_new_shortcuts.end(), shortcuts.begin(), shortcuts.end());
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::_calcShortcuts(Shortcut const& start_edge, NodeID center_node,
		EdgeType direction, ThreadData& td) const -> std::vector<Shortcut> 
{
	struct Target {
		NodeID end_node;
		Shortcut const& end_edge;
	};
	std::vector<Target> targets;
	targets.reserve(_base_graph.getNrOfEdges(center_node, direction));

	NodeID start_node(otherNode(start_edge, !direction));
	uint radius = 0;

	for (auto const& edge: _base_graph.nodeEdges(center_node, direction)) {
		if (edge.tgt == edge.src) continue; /* skip loops */
		auto const end_node = otherNode(edge, direction);
		if (start_node == end_node) continue; /* don't create loops */

		radius = std::max(radius, edge.distance());
		targets.emplace_back(Target { end_node, edge });
	}
	radius += start_edge.distance();

	_calcShortestDists(td, start_node, direction, radius);

	/* abort if start_edge wasn't a shortest path from start_node to center_node */
	if (td.dists[center_node] != start_edge.distance()) return std::vector<Shortcut>();

	std::vector<Shortcut> shortcuts;
	for (auto const& target: targets) {
		/* we know a path within radius - so _calcShortestDists must have found one */
		assert(c::NO_DIST != td.dists[target.end_node]);

		uint center_node_dist(start_edge.distance() + target.end_edge.distance());
		if (td.dists[target.end_node] == center_node_dist) {
			shortcuts.push_back(_createShortcut(start_edge, target.end_edge, direction));
		}
	}

	return shortcuts;
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_calcShortestDists(ThreadData& td, NodeID start_node,
		EdgeType direction, uint radius) const
{
	/* calculates all shortest paths within radius distance from start_node */

	/* clear thread data first */
	td.pq = PQ();
	for (auto node_id: td.reset_dists) {
		td.dists[node_id] = c::NO_DIST;
	}
	td.reset_dists.clear();

	/* now initialize with start node */
	td.pq.push(PQElement(start_node, 0));
	td.dists[start_node] = 0;
	td.reset_dists.push_back(start_node);

	while (!td.pq.empty() && td.pq.top().distance() <= radius) {
		auto top = td.pq.top();
		td.pq.pop();
		if (td.dists[top.node] != top.distance()) continue;

		for (auto const& edge: _base_graph.nodeEdges(top.node, direction)) {
			NodeID tgt_node(otherNode(edge, direction));
			uint new_dist(top.distance() + edge.distance());

			if (new_dist < td.dists[tgt_node]) {
				if (td.dists[tgt_node] == c::NO_DIST) {
					td.reset_dists.push_back(tgt_node);
				}
				td.dists[tgt_node] = new_dist;
				td.pq.push(PQElement(tgt_node, new_dist));
			}
		}
	}
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::_createShortcut(Shortcut const& edge1, Shortcut const& edge2,
		EdgeType direction) const -> Shortcut
{
	/* make sure no "loop" edges are used */
	assert(edge1.src != edge1.tgt && edge2.src != edge2.tgt);

	if (direction == EdgeType::OUT) {
		/* make sure no "loop" edges are created */
		assert(edge1.src != edge2.tgt);
		return make_shortcut(edge1, edge2);
	}
	else {
		/* make sure no "loop" edges are created */
		assert(edge2.src != edge1.tgt);
		return make_shortcut(edge2, edge1);
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_markNeighbours(NodeID node, std::vector<bool>& marked) const
{
	for (uint i(0); i<2; i++) {
		for (auto const& edge: _base_graph.nodeEdges(node, (EdgeType) i)) {
			marked[otherNode(edge, (EdgeType) i)] = true;
		}
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_chooseRemoveNodes(std::vector<NodeID> const& independent_set)
{
	double edge_diff_mean(0);
	for (NodeID node: independent_set) {
		edge_diff_mean += _edge_diffs[node];
	}
	edge_diff_mean /= independent_set.size();
	Print("The average edge difference is " << edge_diff_mean << ".");

	assert(_remove.empty());
	for (NodeID node: independent_set) {
		if (_edge_diffs[node] <= edge_diff_mean) {
			_remove.push_back(node);
			_to_remove[node] = true;
		}
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_chooseAllForRemove(std::vector<NodeID> const& independent_set)
{
	assert(_remove.empty());
	for (NodeID node: independent_set) {
		_remove.push_back(node);
		_to_remove[node] = true;
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_removeNodes(std::vector<NodeID>& nodes)
{
	size_t remaining_nodes(nodes.size());
	size_t i(0);
	while (i < remaining_nodes) {
		NodeID node(nodes[i]);
		if (_to_remove[node]) {
			remaining_nodes--;
			nodes[i] = nodes[remaining_nodes];
			nodes[remaining_nodes] = node;
		}
		else {
			i++;
		}
	}

	nodes.resize(remaining_nodes);
}

/*
 * public
 */

template <typename NodeT, typename EdgeT>
CHConstructor<NodeT, EdgeT>::CHConstructor(CHGraphT& base_graph, uint num_threads)
		:_base_graph(base_graph), _num_threads(num_threads)
{
	if (!_num_threads) {
		_num_threads = 1;
	}

	uint nr_of_nodes(_base_graph.getNrOfNodes());

	_thread_data.resize(_num_threads);
	_edge_diffs.resize(nr_of_nodes);
	_to_remove.resize(nr_of_nodes);

	for (auto& td: _thread_data) {
		td.dists.assign(nr_of_nodes, c::NO_DIST);
		td.reset_dists.reserve(nr_of_nodes);
	}
	_new_shortcuts.reserve(_base_graph.getNrOfEdges());
	_remove.reserve(nr_of_nodes);
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::quickContract(std::vector<NodeID>& nodes, uint max_degree, uint max_rounds)
{
	using namespace std::chrono;

	Print("\nStarting the quick_contraction of nodes with degree smaller than " << max_degree << ".\n");

	for (uint round(1); round <= max_rounds; ++round) {
		steady_clock::time_point t1 = steady_clock::now();
		Print("Starting round " << round);
		Debug("Initializing the vectors for a new round.");
		_initVectors();

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		std::sort(nodes.begin(), nodes.end(), CompInOutProduct(_base_graph));

		Debug("Constructing the independent set.");
		auto independent_set = calcIndependentSet(nodes, max_degree);
		Print("The independent set has size " << independent_set.size() << ".");

		if (independent_set.empty()) break;

		Debug("Quick-contracting all the nodes in the independent set.");
		uint size(independent_set.size());
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < size; i++) {
			uint node(independent_set[i]);
			_quickContract(node);
		}
		Print("Number of possible new Shortcuts: " << _new_shortcuts.size());

		Debug("Remove the nodes with low edge difference.");
		_chooseAllForRemove(independent_set);
		_removeNodes(nodes);
		Print("Removed " << _remove.size() << " nodes with low edge difference.");

		Debug("Restructuring the graph.");
		_base_graph.restructure(_remove, _to_remove, _new_shortcuts);

		Print("Graph info:");
		_base_graph.printInfo(nodes);

		duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - t1);
		Print("Round took " << time_span.count() << " seconds.\n");
		Unused(time_span);
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::contract(std::vector<NodeID>& nodes)
{
	using namespace std::chrono;

	Print("\nStarting the contraction of " << nodes.size() << " nodes.\n");

	for (uint round(1); !nodes.empty(); ++round) {
		steady_clock::time_point t1 = steady_clock::now();
		Print("Starting round " << round);
		Debug("Initializing the vectors for a new round.");
		_initVectors();

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		std::sort(nodes.begin(), nodes.end(), CompInOutProduct(_base_graph));

		Debug("Constructing the independent set.");
		auto independent_set = calcIndependentSet(nodes);
		Print("The independent set has size " << independent_set.size() << ".");

		Debug("Contracting all the nodes in the independent set.");
		uint size(independent_set.size());
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < size; i++) {
			uint node(independent_set[i]);
			_contract(node);
		}
		Print("Number of possible new Shortcuts: " << _new_shortcuts.size());

		Debug("Remove the nodes with low edge difference.");
		_chooseRemoveNodes(independent_set);
		_removeNodes(nodes);
		Print("Removed " << _remove.size() << " nodes with low edge difference.");

		Debug("Restructuring the graph.");
		_base_graph.restructure(_remove, _to_remove, _new_shortcuts);

		Print("Graph info:");
		_base_graph.printInfo(nodes);

		duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - t1);
		Print("Round took " << time_span.count() << " seconds.\n");
		Unused(time_span);
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::contract(std::vector<NodeID>& nodes, Prioritizer& prioritizer)
{
	using namespace std::chrono;

	Print("\nStarting the contraction of " << nodes.size() << " nodes.\n");

	prioritizer.init(nodes);

	uint round(1);
	while (prioritizer.hasNodesLeft()) {
		steady_clock::time_point t1 = steady_clock::now();
		Print("Starting round " << round);
		Debug("Initializing the vectors for a new round.");
		_initVectors();

		Debug("Calculating list of nodes to be contracted next.");
		auto next_nodes(prioritizer.extractNextNodes());
		Print("There are " << next_nodes.size() << " nodes to be contracted in this round.");

		Debug("Contracting all the nodes in the independent set.");
		uint size(next_nodes.size());
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < size; i++) {
			uint node(next_nodes[i]);
			_contract(node);
		}
		Print("Number of new Shortcuts: " << _new_shortcuts.size());

		Debug("Mark nodes for removal from graph.");
		_chooseAllForRemove(next_nodes);
		Print("Marked " << _remove.size() << " nodes.");

		Debug("Restructuring the graph.");
		_base_graph.restructure(_remove, _to_remove, _new_shortcuts);

		Print("Graph info:");
		_base_graph.printInfo();

		duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - t1);
		Print("Round took " << time_span.count() << " seconds.\n");
		Unused(time_span);

		round++;
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::rebuildCompleteGraph()
{
	Print("Restoring edges from contracted nodes.");

	_base_graph.rebuildCompleteGraph();
}

template <typename NodeT, typename EdgeT>
std::vector<NodeID> CHConstructor<NodeT, EdgeT>::calcIndependentSet(std::vector<NodeID> const& nodes,
		uint max_degree) const
{
	std::vector<NodeID> independent_set;
	std::vector<bool> marked(_base_graph.getNrOfNodes(), false);
	independent_set.reserve(nodes.size());

	for (NodeID node: nodes) {
		if (!marked[node] && max_degree >= _base_graph.getNrOfEdges(node)) {
			marked[node] = true;
			_markNeighbours(node, marked);
			independent_set.push_back(node);
		}
	}
	return independent_set;
}

template <typename NodeT, typename EdgeT>
int CHConstructor<NodeT, EdgeT>::calcEdgeDiff(NodeID node) const
{
	auto shortcuts(getShortcutsOfContracting(node));
	return shortcuts.size() - (int) _base_graph.getNrOfEdges(node);
}

template <typename NodeT, typename EdgeT>
std::vector<int> CHConstructor<NodeT, EdgeT>::calcEdgeDiffs(std::vector<NodeID> const& nodes) const
{
	std::vector<int> edge_diffs(nodes.size());
	auto shortcuts(getShortcutsOfContracting(nodes));

	uint size(nodes.size());
	#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
	for (uint i = 0; i<size; i++) {
		edge_diffs[i] = shortcuts[i].size() - (int) _base_graph.getNrOfEdges(nodes[i]);
	}

	return edge_diffs;
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::getShortcutsOfContracting(NodeID node) const -> std::vector<Shortcut>
{
	ThreadData td;
	return _contract(node, td);
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::getShortcutsOfContracting(std::vector<NodeID> const& nodes) const
		-> std::vector<std::vector<Shortcut>>
{
	std::vector<std::vector<Shortcut>> shortcuts(nodes.size());

	/* init thread data */
	std::vector<ThreadData> thread_data;
	thread_data.resize(_num_threads);
	auto nr_of_nodes(_base_graph.getNrOfNodes());
	for (auto& td: thread_data) {
		td.dists.assign(nr_of_nodes, c::NO_DIST);
		td.reset_dists.reserve(nr_of_nodes);
	}

	/* calc shortcuts */
	uint size(nodes.size());
	#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
	for (uint i = 0; i < size; i++) {
		uint node(nodes[i]);

		auto& td(thread_data[omp_get_thread_num()]);
		shortcuts[i] = _contract(node, td);
	}

	return shortcuts;
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::getShortcutsOfQuickContracting(NodeID node) const -> std::vector<Shortcut>
{
	std::vector<Shortcut> shortcuts;
	for (auto const& in_edge: _base_graph.nodeEdges(node, EdgeType::IN)) {
		if (in_edge.tgt == in_edge.src) continue; /* skip loops */
		for (auto const& out_edge: _base_graph.nodeEdges(node, EdgeType::OUT)) {
			if (out_edge.tgt == out_edge.src) continue; /* skip loops */
			if (in_edge.src != out_edge.tgt) { /* don't create loops */
				shortcuts.push_back(_createShortcut(in_edge, out_edge));
			}
		}
	}

	return shortcuts;
}

template <typename NodeT, typename EdgeT>
auto CHConstructor<NodeT, EdgeT>::getShortcutsOfQuickContracting(std::vector<NodeID> const& nodes) const -> std::vector<std::vector<Shortcut>>
{
	std::vector<std::vector<Shortcut>> shortcuts(nodes.size());

	/* calc shortcuts */
	uint size(nodes.size());
	#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
	for (uint i = 0; i < size; i++) {
		shortcuts[i] = getShortcutsOfQuickContracting(nodes[i]);
	}

	return shortcuts;
}

}
