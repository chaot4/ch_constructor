#pragma once

#include "defs.h"
#include "nodes_and_edges.h"
#include "graph.h"
#include "chgraph.h"

#include <chrono>
#include <queue>
#include <mutex>
#include <omp.h>

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
		typedef SCGraph<NodeT, EdgeT> CHGraph;

		struct CompInOutProduct;
		struct PQElement;
		typedef std::priority_queue<
				PQElement, std::vector<PQElement>, std::greater<PQElement> > PQ;

		CHGraph& _base_graph;

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
		std::vector<NodeID> _delete;
		std::vector<bool> _to_delete;
		std::mutex _new_shortcuts_mutex;


		void _initVectors();
		void _contract(NodeID node);
		void _quickContract(NodeID node);
		uint _calcShortcuts(Shortcut const& start_edge, NodeID center_node,
				EdgeType direction);
		void _calcShortestDists(ThreadData& td, NodeID start_node, EdgeType direction, uint radius);
		void _handleNextPQElement(EdgeType direction);
		void _createShortcut(Shortcut const& edge1, Shortcut const& edge2,
				EdgeType direction = EdgeType::OUT);

		std::vector<NodeID> _calcIndependentSet(std::list<NodeID> const& nodes,
				uint max_degree = MAX_UINT);
		void _markNeighbours(NodeID node, std::vector<bool>& marked);

		void _chooseDeleteNodes(std::vector<NodeID> const& independent_set);
		void _chooseAllForDelete(std::vector<NodeID> const& independent_set);
		void _deleteNodes(std::list<NodeID>& nodes);
	public:
		CHConstructor(CHGraph& base_graph, uint num_threads = 1);

		void quick_contract(std::list<NodeID>& nodes, uint max_degree,
				uint max_rounds);
		void contract(std::list<NodeID>& nodes);
		void rebuildCompleteGraph();

		friend void unit_tests::testCHConstructor();
};

/*
 * private
 */

template <typename NodeT, typename EdgeT>
struct CHConstructor<NodeT, EdgeT>::CompInOutProduct
{
	CHGraph const& g;

	CompInOutProduct(CHGraph const& g)
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
	_delete.clear();
	_to_delete.assign(_base_graph.getNrOfNodes(), false);
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_contract(NodeID node)
{
	EdgeType search_direction;

	if (_base_graph.getNrOfEdges(node, EdgeType::IN) <= _base_graph.getNrOfEdges(node, EdgeType::OUT)) {
		search_direction = EdgeType::OUT;
	}
	else {
		search_direction = EdgeType::IN;
	}

	int nr_new_edges(0);
	for (auto const& edge: _base_graph.nodeEdges(node, !search_direction)) {
		if (edge.tgt == edge.src) continue; /* skip loops */
		nr_new_edges += _calcShortcuts(edge, node, search_direction);
	}

	_edge_diffs[node] = nr_new_edges - (int) _base_graph.getNrOfEdges(node);
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_quickContract(NodeID node)
{
	for (auto const& in_edge: _base_graph.nodeEdges(node, EdgeType::IN)) {
		if (in_edge.tgt == in_edge.src) continue; /* skip loops */
		for (auto const& out_edge: _base_graph.nodeEdges(node, EdgeType::OUT)) {
			if (out_edge.tgt == out_edge.src) continue; /* skip loops */
			if (in_edge.src != out_edge.tgt) { /* don't create loops */
				_createShortcut(in_edge, out_edge);
			}
		}
	}
}

template <typename NodeT, typename EdgeT>
uint CHConstructor<NodeT, EdgeT>::_calcShortcuts(Shortcut const& start_edge, NodeID center_node,
		EdgeType direction)
{
	ThreadData& td(_myThreadData());

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
	if (td.dists[center_node] != start_edge.distance()) return 0;

	uint nr_new_edges(0);
	for (auto const& target: targets) {
		/* we know a path within radius - so _calcShortestDists must have found one */
		assert(c::NO_DIST != td.dists[target.end_node]);

		uint center_node_dist(start_edge.distance() + target.end_edge.distance());
		if (td.dists[target.end_node] == center_node_dist) {
			_createShortcut(start_edge, target.end_edge, direction);
			nr_new_edges++;
		}
	}

	return nr_new_edges;
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_calcShortestDists(ThreadData& td, NodeID start_node, EdgeType direction, uint radius)
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
void CHConstructor<NodeT, EdgeT>::_createShortcut(Shortcut const& edge1, Shortcut const& edge2,
		EdgeType direction)
{
	/* make sure no "loop" edges are used */
	assert(edge1.src != edge1.tgt && edge2.src != edge2.tgt);

	std::unique_lock<std::mutex> lock(_new_shortcuts_mutex);
	if (direction == EdgeType::OUT) {
		/* make sure no "loop" edges are created */
		assert(edge1.src != edge2.tgt);
		_new_shortcuts.push_back(make_shortcut(edge1, edge2));
	}
	else {
		/* make sure no "loop" edges are created */
		assert(edge2.src != edge1.tgt);
		_new_shortcuts.push_back(make_shortcut(edge2, edge1));
	}
}

template <typename NodeT, typename EdgeT>
std::vector<NodeID> CHConstructor<NodeT, EdgeT>::_calcIndependentSet(std::list<NodeID> const& nodes,
		uint max_degree)
{
	std::vector<NodeID> independent_set;
	std::vector<bool> marked(_base_graph.getNrOfNodes(), false);
	independent_set.reserve(nodes.size());

	for (auto it(nodes.begin()), end(nodes.end()); it != end; it++) {
		if (!marked[*it] && max_degree >= _base_graph.getNrOfEdges(*it)) {
			marked[*it] = true;
			_markNeighbours(*it, marked);
			independent_set.push_back(*it);
		}
	}
	return independent_set;
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_markNeighbours(NodeID node, std::vector<bool>& marked)
{
	for (uint i(0); i<2; i++) {
		for (auto const& edge: _base_graph.nodeEdges(node, (EdgeType) i)) {
			marked[otherNode(edge, (EdgeType) i)] = true;
		}
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_chooseDeleteNodes(std::vector<NodeID> const& independent_set)
{
	double edge_diff_mean(0);
	for (uint i(0), size(independent_set.size()); i<size; i++) {
		edge_diff_mean += _edge_diffs[independent_set[i]];
	}
	edge_diff_mean /= independent_set.size();
	Print("The average edge difference is " << edge_diff_mean << ".");

	assert(_delete.empty());
	for (uint i(0), size(independent_set.size()); i<size; i++) {
		NodeID node(independent_set[i]);
		if (_edge_diffs[node] <= edge_diff_mean) {
			_delete.push_back(node);
			_to_delete[node] = true;
		}
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_chooseAllForDelete(std::vector<NodeID> const& independent_set)
{
	assert(_delete.empty());
	for (uint i(0), size(independent_set.size()); i<size; i++) {
		NodeID node(independent_set[i]);
		_delete.push_back(node);
		_to_delete[node] = true;
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::_deleteNodes(std::list<NodeID>& nodes)
{
	auto it(nodes.begin());
	while (it != nodes.end()) {
		if (_to_delete[*it]) {
			it = nodes.erase(it);
		}
		else {
			it++;
		}
	}
}

/*
 * public
 */

template <typename NodeT, typename EdgeT>
CHConstructor<NodeT, EdgeT>::CHConstructor(CHGraph& base_graph, uint num_threads)
		:_base_graph(base_graph), _num_threads(num_threads)
{
	if (!_num_threads) {
		_num_threads = 1;
	}

	uint nr_of_nodes(_base_graph.getNrOfNodes());

	_thread_data.resize(_num_threads);
	_edge_diffs.resize(nr_of_nodes);
	_to_delete.resize(nr_of_nodes);

	for (auto& td: _thread_data) {
		td.dists.assign(nr_of_nodes, c::NO_DIST);
		td.reset_dists.reserve(nr_of_nodes);
	}
	_new_shortcuts.reserve(_base_graph.getNrOfEdges());
	_delete.reserve(nr_of_nodes);
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::quick_contract(std::list<NodeID>& nodes, uint max_degree, uint max_rounds)
{
	using namespace std::chrono;

	Print("\nStarting the quick_contraction of nodes with degree smaller than " << max_degree << ".\n");

	for (uint round(1); round <= max_rounds; ++round) {
		steady_clock::time_point t1 = steady_clock::now();
		Print("Starting round " << round);
		Debug("Initializing the vectors for a new round.");
		_initVectors();

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		nodes.sort<CompInOutProduct>(CompInOutProduct(_base_graph));

		Debug("Constructing the independent set.");
		auto independent_set = _calcIndependentSet(nodes, max_degree);
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

		Debug("Delete the nodes with low edge difference.");
		_chooseAllForDelete(independent_set);
		_deleteNodes(nodes);
		Print("Deleted " << _delete.size() << " nodes with low edge difference.");

		Debug("Restructuring the graph.");
		_base_graph.restructure(_delete, _to_delete, _new_shortcuts);

		Debug("Graph info:");
		_base_graph.printInfo(nodes);

		duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - t1);
		Print("Round took " << time_span.count() << " seconds.\n");
		Unused(time_span);
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::contract(std::list<NodeID>& nodes)
{
	using namespace std::chrono;

	Print("\nStarting the contraction of " << nodes.size() << " nodes.\n");

	for (uint round(1); !nodes.empty(); ++round) {
		steady_clock::time_point t1 = steady_clock::now();
		Print("Starting round " << round);
		Debug("Initializing the vectors for a new round.");
		_initVectors();

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		nodes.sort<CompInOutProduct>(CompInOutProduct(_base_graph));

		Debug("Constructing the independent set.");
		auto independent_set = _calcIndependentSet(nodes);
		Print("The independent set has size " << independent_set.size() << ".");

		Debug("Contracting all the nodes in the independent set.");
		uint size(independent_set.size());
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < size; i++) {
			uint node(independent_set[i]);
			_contract(node);
		}
		Print("Number of possible new Shortcuts: " << _new_shortcuts.size());

		Debug("Delete the nodes with low edge difference.");
		_chooseDeleteNodes(independent_set);
		_deleteNodes(nodes);
		Print("Deleted " << _delete.size() << " nodes with low edge difference.");

		Debug("Restructuring the graph.");
		_base_graph.restructure(_delete, _to_delete, _new_shortcuts);

		Debug("Graph info:");
		_base_graph.printInfo(nodes);

		duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - t1);
		Print("Round took " << time_span.count() << " seconds.\n");
		Unused(time_span);
	}
}

template <typename NodeT, typename EdgeT>
void CHConstructor<NodeT, EdgeT>::rebuildCompleteGraph()
{
	Print("Restoring edges from contracted nodes.");

	_base_graph.rebuildCompleteGraph();
}


}
