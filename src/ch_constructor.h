#ifndef _CH_CONSTRUCTOR
#define _CH_CONSTRUCTOR

#include "defs.h"
#include "nodes_and_edges.h"
#include "graph.h"
#include "chgraph.h"

#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <limits>
#include <mutex>
#include <omp.h>

namespace unit_tests
{
	void testCHConstructor();
}

namespace
{
	uint MAX_UINT(std::numeric_limits<uint>::max());
}

template <typename Node, typename Edge>
class CHConstructor{
	private:
		typedef CHNode<Node> LvlNode;
		typedef CHEdge<Edge> Shortcut;
		typedef SCGraph<Node, Edge> CHGraph;

		struct CompInOutProduct;
		struct PQElement;
		typedef std::priority_queue<
				PQElement, std::vector<PQElement>, std::greater<PQElement> > PQ;

		CHGraph& _base_graph;

		uint _num_threads;
		uint _myThreadNum();
		void _resetThreadData();

		std::vector<Shortcut> _new_shortcuts;
		std::vector<int> _edge_diffs;
		std::vector<NodeID> _delete;
		std::vector<bool> _to_delete;
		std::mutex _new_shortcuts_mutex;

		std::vector<PQ> _pq;
		std::vector< std::vector<uint> > _dists;
		std::vector< std::vector<uint> > _reset_dists;

		void _initVectors(bool quick_version = false);
		void _contract(NodeID node);
		void _quickContract(NodeID node);
		uint _calcShortcuts(Shortcut const& start_edge, NodeID center_node,
				EdgeType direction);
		void _handleNextPQElement(EdgeType direction);
		void _createShortcut(Shortcut const& edge1, Shortcut const& edge2,
				EdgeType direction = OUT);

		void _calcIndependentSet(std::list<NodeID>& nodes,
				std::vector<NodeID>& independent_set,
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
		SCGraph<Node, Edge> const& getCHGraph();

		friend void unit_tests::testCHConstructor();
};

/*
 * private
 */

template <typename Node, typename Edge>
struct CHConstructor<Node, Edge>::CompInOutProduct
{
	CHGraph const& g;

	CompInOutProduct(CHGraph const& g)
		: g(g) {}

	bool operator()(NodeID node1, NodeID node2) const
	{
		uint edge_product1(g.getNrOfEdges(node1, IN)
				* g.getNrOfEdges(node1, OUT));
		uint edge_product2(g.getNrOfEdges(node2, IN)
				* g.getNrOfEdges(node2, OUT));

		return edge_product1 < edge_product2;
	}
};

template <typename Node, typename Edge>
struct CHConstructor<Node, Edge>::PQElement
{
	NodeID id;
	uint dist;

	PQElement(NodeID id, uint dist)
		: id(id), dist(dist) {}

	bool operator>(PQElement const& other) const
	{
		return dist > other.dist;
	}
};

template <typename Node, typename Edge>
uint CHConstructor<Node, Edge>::_myThreadNum()
{
	return omp_get_thread_num();
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_resetThreadData()
{
	uint t(_myThreadNum());

	_pq[t] = PQ();

	for (uint i(0); i<_reset_dists[t].size(); i++) {
		_dists[t][_reset_dists[t][i]] = MAX_UINT;
	}
	_reset_dists[t].clear();
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_initVectors(bool quick_version)
{
	_new_shortcuts.clear();
	_delete.clear();
	_to_delete.assign(_base_graph.getNrOfNodes(), false);

	if (!quick_version) {
		for (uint i(0); i<_num_threads; i++) {
			_pq[i] = PQ();
			_dists[i].assign(_base_graph.getNrOfNodes(), MAX_UINT);
			_reset_dists[i].clear();
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_contract(NodeID node)
{
	EdgeType search_direction;

	if (_base_graph.getNrOfEdges(node, IN) <= _base_graph.getNrOfEdges(node, OUT)) {
		search_direction = OUT;
	}
	else {
		search_direction = IN;
	}

	uint nr_new_edges(0);
	typename CHGraph::EdgeIt edge_it(_base_graph, node, !search_direction);
	while (edge_it.hasNext()) {
		_resetThreadData();
		nr_new_edges += _calcShortcuts(edge_it.getNext(), node, search_direction);
	}

	uint edge_diff(nr_new_edges - _base_graph.getNrOfEdges(node));
	_edge_diffs[node] = edge_diff;
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_quickContract(NodeID node)
{
	typename CHGraph::EdgeIt in_it(_base_graph, node, IN);
	while (in_it.hasNext()) {
		Shortcut const& in_edge(in_it.getNext());
		typename CHGraph::EdgeIt out_it(_base_graph, node, OUT);
		while (out_it.hasNext()) {
			Shortcut const& out_edge(out_it.getNext());
			if (in_edge.src != out_edge.tgt) {
				_createShortcut(in_edge, out_edge);
			}
		}
	}
}

template <typename Node, typename Edge>
uint CHConstructor<Node, Edge>::_calcShortcuts(Shortcut const& start_edge, NodeID center_node,
		EdgeType direction)
{
	uint t(_myThreadNum());

	NodeID start_node(start_edge.otherNode(!direction));

	std::vector<NodeID> end_nodes;
	std::vector<Shortcut const*> end_edges;
	end_nodes.reserve(_base_graph.getNrOfEdges(center_node, direction));
	end_edges.reserve(_base_graph.getNrOfEdges(center_node, direction));

	typename CHGraph::EdgeIt edge_it(_base_graph, center_node, direction);
	while (edge_it.hasNext()) {
		Shortcut const& edge(edge_it.getNext());
		end_edges.push_back(&edge);
		end_nodes.push_back(edge.otherNode(direction));
	}

	_pq[t].push(PQElement(start_node, 0));
	_dists[t][start_node] = 0;
	_reset_dists[t].push_back(start_node);

	uint nr_new_edges(0);
	for (uint i(0); i<end_nodes.size(); i++) {

		while (_dists[t][end_nodes[i]] > _pq[t].top().dist) {
			_handleNextPQElement(direction);
		}

		uint center_node_dist(start_edge.dist + end_edges[i]->dist);
		if (_dists[t][end_nodes[i]] == center_node_dist) {
			_createShortcut(start_edge, *end_edges[i], direction);
			nr_new_edges++;
		}
	}

	return nr_new_edges;
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_handleNextPQElement(EdgeType direction)
{
	uint t(_myThreadNum());

	NodeID node(_pq[t].top().id);
	uint dist(_pq[t].top().dist);

	if (_dists[t][node] == dist) {

		typename CHGraph::EdgeIt edge_it(_base_graph, node, direction);
		while (edge_it.hasNext()) {

			Shortcut const& edge(edge_it.getNext());
			NodeID tgt_node(edge.otherNode(direction));
			uint new_dist(dist + edge.dist);

			if (new_dist < _dists[t][tgt_node]) {

				if (_dists[t][tgt_node] == MAX_UINT) {
					_reset_dists[t].push_back(tgt_node);
				}
				_dists[t][tgt_node] = new_dist;
				_pq[t].push(PQElement(tgt_node, new_dist));
			}
		}
	}

	_pq[t].pop();
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_createShortcut(Shortcut const& edge1, Shortcut const& edge2,
		EdgeType direction)
{
	std::unique_lock<std::mutex> lock(_new_shortcuts_mutex);
	if (direction == OUT) {
		_new_shortcuts.push_back(edge1.concat(edge2));
		assert(edge1.tgt == edge2.src);
		assert(_new_shortcuts.back().center_node == edge1.tgt);
	}
	else {
		_new_shortcuts.push_back(edge2.concat(edge1));
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_calcIndependentSet(std::list<NodeID>& nodes,
		std::vector<NodeID>& independent_set, uint max_degree)
{
	std::vector<bool> marked(_base_graph.getNrOfNodes(), false);
	independent_set.reserve(nodes.size());

	for (auto it(nodes.begin()); it != nodes.end(); it++) {
		if (!marked[*it] && max_degree >= _base_graph.getNrOfEdges(*it)) {
			marked[*it] = true;
			_markNeighbours(*it, marked);
			independent_set.push_back(*it);
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_markNeighbours(NodeID node, std::vector<bool>& marked)
{
	for (uint i(0); i<2; i++) {
		typename CHGraph::EdgeIt edge_it(_base_graph, node, (EdgeType) i);
		while (edge_it.hasNext()) {
			Shortcut const& edge(edge_it.getNext());
			marked[edge.otherNode((EdgeType) i)] = true;
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_chooseDeleteNodes(std::vector<NodeID> const& independent_set)
{
	double edge_diff_mean(0);
	for (uint i(0); i<independent_set.size(); i++) {
		edge_diff_mean += _edge_diffs[independent_set[i]];
	}
	edge_diff_mean /= independent_set.size();
	Print("The average edge difference is " << edge_diff_mean << ".");

	assert(_delete.empty());
	for (uint i(0); i<independent_set.size(); i++) {
		NodeID node(independent_set[i]);
		if (_edge_diffs[node] <= edge_diff_mean) {
			_delete.push_back(node);
			_to_delete[node] = true;
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_chooseAllForDelete(std::vector<NodeID> const& independent_set)
{
	assert(_delete.empty());
	for (uint i(0); i<independent_set.size(); i++) {
		NodeID node(independent_set[i]);
		_delete.push_back(node);
		_to_delete[node] = true;
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_deleteNodes(std::list<NodeID>& nodes)
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

template <typename Node, typename Edge>
CHConstructor<Node, Edge>::CHConstructor(CHGraph& base_graph, uint num_threads)
		:_base_graph(base_graph), _num_threads(num_threads)
{
	if (!_num_threads) {
		_num_threads = 1;
	}

	uint nr_of_nodes(_base_graph.getNrOfNodes());

	_pq.resize(_num_threads);
	_dists.resize(_num_threads);
	_reset_dists.resize(_num_threads);
	_edge_diffs.resize(nr_of_nodes);
	_to_delete.resize(nr_of_nodes);

	for (uint i(0); i<_num_threads; i++) {
		_dists[i].reserve(nr_of_nodes);
		_reset_dists[i].reserve(nr_of_nodes);
	}
	_new_shortcuts.reserve(_base_graph.getNrOfEdges());
	_delete.reserve(nr_of_nodes);
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::quick_contract(std::list<NodeID>& nodes, uint max_degree, uint max_rounds)
{
	Print("\nStarting the quick_contraction of nodes with degree smaller than " << max_degree << ".");

	bool nodes_left(true);
	uint round(1);
	while (nodes_left && round <= max_rounds) {
		Print("\nStarting round " << round);
		Print("Initializing the vectors for a new round.");
		_initVectors(true);

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		nodes.sort<CompInOutProduct>(CompInOutProduct(_base_graph));

		Print("Constructing the independent set.");
		std::vector<NodeID> independent_set;
		_calcIndependentSet(nodes, independent_set, max_degree);
		Print("The independent set has size " << independent_set.size() << ".");

		if (!independent_set.empty()) {
			Print("Quick-contracting all the nodes in the independent set.");
			#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
			for (uint i = 0; i < independent_set.size(); i++) {
				uint node(independent_set[i]);
				_quickContract(node);
			}
			Print("Number of possible new Shortcuts: " << _new_shortcuts.size());

			_chooseAllForDelete(independent_set);
			_deleteNodes(nodes);
			Print("Deleted " << _delete.size() << " nodes.");

			Print("Restructuring the graph.");
			_base_graph.restructure(_delete, _to_delete, _new_shortcuts);

			Print("Graph info:");
			_base_graph.printInfo(nodes);

			round++;
		}
		else {
			nodes_left = false;
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::contract(std::list<NodeID>& nodes)
{
	Print("\nStarting the contraction of " << nodes.size() << " nodes.");

	uint round(1);
	while (!nodes.empty()) {
		Print("\nStarting round " << round);
		Print("Initializing the vectors for a new round.");
		_initVectors();

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		nodes.sort<CompInOutProduct>(CompInOutProduct(_base_graph));

		Print("Constructing the independent set.");
		std::vector<NodeID> independent_set;
		_calcIndependentSet(nodes, independent_set);
		Print("The independent set has size " << independent_set.size() << ".");

		Print("Contracting all the nodes in the independent set.");
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < independent_set.size(); i++) {
			uint node(independent_set[i]);
			_contract(node);
		}
		Print("Number of possible new Shortcuts: " << _new_shortcuts.size());

		Print("Delete the nodes with low edge difference.");
		_chooseDeleteNodes(independent_set);
		_deleteNodes(nodes);
		Print("Deleted " << _delete.size() << " nodes.");

		Print("Restructuring the graph.");
		_base_graph.restructure(_delete, _to_delete, _new_shortcuts);

		Print("Graph info:");
		_base_graph.printInfo(nodes);

		round++;
	}
}

template <typename Node, typename Edge>
SCGraph<Node, Edge> const& CHConstructor<Node, Edge>::getCHGraph()
{
	Print("Building the CHGraph.");

	_base_graph.buildCHGraph();
	return _base_graph;
}

#endif
