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
	int MAX_UINT(std::numeric_limits<uint>::max());
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
		std::vector<bool> _delete;
		std::mutex _new_shortcuts_mutex;

		std::vector<PQ> _pq;
		std::vector< std::vector<uint> > _dists;
		std::vector< std::vector<uint> > _reset_dists;

		void _initVectors();
		void _contract(NodeID center_node);
		void _calcShortcuts(Shortcut const& start_edge, NodeID center_node,
				EdgeType direction);
		void _handleNextPQElement(EdgeType direction);
		void _createShortcut(Shortcut const& edge1, Shortcut const& edge2,
				EdgeType direction);

		void _extractIndependentSet(std::list<NodeID>& nodes,
				std::vector<NodeID>& independent_set);
		void _markNeighbours(NodeID node, std::vector<bool>& marked);
	public:
		CHConstructor(CHGraph& base_graph, uint num_threads = 1);

		void contract(std::list<NodeID> nodes);
		void getCHGraph(CHGraph& ch_graph);

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
void CHConstructor<Node, Edge>::_initVectors()
{
	_new_shortcuts.clear();
	_delete.assign(_base_graph.getNrOfNodes(), false);

	for (uint i(0); i<_num_threads; i++) {
		_pq[i] = PQ();
		_dists[i].assign(_base_graph.getNrOfNodes(), MAX_UINT);
		_reset_dists[i].clear();
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

	typename CHGraph::EdgeIt edge_it(_base_graph, node, !search_direction);
	while (edge_it.hasNext()) {
		_resetThreadData();
		_calcShortcuts(edge_it.getNext(), node, search_direction);
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_calcShortcuts(Shortcut const& start_edge, NodeID center_node,
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

	for (uint i(0); i<end_nodes.size(); i++) {

		while (_dists[t][end_nodes[i]] > _pq[t].top().dist) {
			_handleNextPQElement(direction);
		}

		uint center_node_dist(start_edge.dist + end_edges[i]->dist);
		if (_dists[t][end_nodes[i]] == center_node_dist) {
			_createShortcut(start_edge, *end_edges[i], direction);
		}
	}
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

				_dists[t][tgt_node] = new_dist;
				_reset_dists[t].push_back(tgt_node);
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
	}
	else {
		_new_shortcuts.push_back(edge2.concat(edge1));
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_extractIndependentSet(std::list<NodeID>& nodes,
		std::vector<NodeID>& independent_set)
{
	std::vector<bool> marked(_base_graph.getNrOfNodes(), false);
	independent_set.reserve(nodes.size());

	auto it(nodes.begin());
	while (it != nodes.end()) {
		if (!marked[*it]) {
			marked[*it] = true;
			_markNeighbours(*it, marked);
			
			independent_set.push_back(*it);
			it = nodes.erase(it);
		}
		else {
			it++;
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

/*
 * public
 */

template <typename Node, typename Edge>
CHConstructor<Node, Edge>::CHConstructor(CHGraph& base_graph,
		uint num_threads)
		:_base_graph(base_graph), _num_threads(num_threads)
{
	if (!_num_threads) {
		_num_threads = 1;
	}

	_pq.resize(_num_threads);
	_dists.resize(_num_threads);
	_reset_dists.resize(_num_threads);
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::contract(std::list<NodeID> nodes)
{
	for (uint i(0); i<_num_threads; i++) {
		_new_shortcuts.reserve(_base_graph.getNrOfEdges());
		_dists[i].reserve(_base_graph.getNrOfNodes());
		_reset_dists[i].reserve(_base_graph.getNrOfNodes());
	}

	Print("\nStarting the contraction of " << nodes.size() << " nodes.");

	while (!nodes.empty()) {

		Print("\nInitializing the vectors for a new round.");
		_initVectors();

		Print("Sorting the remaining " << nodes.size() << " nodes.");
		nodes.sort<CompInOutProduct>(CompInOutProduct(_base_graph));

		Print("Constructing the independent set.");
		std::vector<NodeID> independent_set;
		_extractIndependentSet(nodes, independent_set);
		Print("The independent set has size " << independent_set.size() << ".");

		Print("Contracting all the nodes in the independent set.");
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < independent_set.size(); i++) {
			_contract(independent_set[i]);
			_delete[independent_set[i]] = true;
		}

		Print("Number of new Shortcuts: " << _new_shortcuts.size());

		Print("Restructuring the graph.");
		_base_graph.restructure(_delete, _new_shortcuts);
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::getCHGraph(CHGraph& ch_graph)
{

}

#endif
