#ifndef _CH_CONSTRUCTOR
#define _CH_CONSTRUCTOR

#include "defs.h"
#include "nodes_and_edges.h"
#include "graph.h"

#include <vector>
#include <list>
#include <algorithm>

namespace unit_tests
{
	void testCHConstructor();
}

struct InOutProduct
{
	Graph<Node, Edge> const& g;

	InOutProduct(Graph<Node, Edge> const& g)
		: g(g) {}

	bool operator()(NodeID node1, NodeID node2) const
	{
		uint edge_product1(g.getNrOfEdges(node1, IN) * g.getNrOfEdges(node1, OUT));
		uint edge_product2(g.getNrOfEdges(node2, IN) * g.getNrOfEdges(node2, OUT));

		return edge_product1 < edge_product2;
	}
};

template <typename Node, typename Edge>
class CHConstructor{
	private:
		Graph<Node, Edge> const& _base_graph;
		uint _num_threads;

		std::vector< std::vector<CHEdge<Edge> > > _new_shortcuts;

		void _contract(NodeID center_node);
		void _calcShortcuts(NodeID neighbour, NodeID center_node, EdgeType direction);

		void _extractIndependentSet(std::list<NodeID>& nodes,
				std::vector<NodeID>& independent_set);
		void _markNeighbours(NodeID node, std::vector<bool>& marked);
	public:
		CHConstructor(Graph<Node, Edge> const& base_graph, uint num_threads = 1);

		void contract(std::list<NodeID> nodes);
		void getCHGraph(CHGraph& ch_graph);

		friend void unit_tests::testCHConstructor();
};

/*
 * private
 */

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

	typename Graph<Node, Edge>::EdgeIt edge_it(_base_graph, node, !search_direction);
	while (edge_it.hasNext()) {
		_calcShortcuts(edge_it.getNext().otherNode(!search_direction),
				node, search_direction);
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_calcShortcuts(NodeID neighbour, NodeID center_node,
		EdgeType direction)
{
	// TODO
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_extractIndependentSet(std::list<NodeID>& nodes,
		std::vector<NodeID>& independent_set)
{
	std::vector<bool> marked(_base_graph.getNrOfNodes(), false);

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
		typename Graph<Node, Edge>::EdgeIt edge_it(_base_graph, node, (EdgeType) i);
		while (edge_it.hasNext()) {
			Edge const& edge(edge_it.getNext());
			marked[edge.otherNode((EdgeType) i)] = true;
		}
	}
}

/*
 * public
 */

template <typename Node, typename Edge>
CHConstructor<Node, Edge>::CHConstructor(Graph<Node, Edge> const& base_graph,
		uint num_threads)
		:_base_graph(base_graph), _num_threads(num_threads)
{
	if (!_num_threads) {
		_num_threads = 1;
	}

	_new_shortcuts.resize(2);
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::contract(std::list<NodeID> nodes)
{
	Print("\nStarting the contraction of the remaining " << nodes.size() << " nodes.");

	while (!nodes.empty()) {
		Print("\nSorting the remaining " << nodes.size() << " nodes.");
		nodes.sort<InOutProduct>(InOutProduct(_base_graph));

		Print("Constructing the independent set.");
		std::vector<NodeID> independent_set;
		_extractIndependentSet(nodes, independent_set);
		Print("The independent set has size " << independent_set.size() << ".");

		Print("Contracting all the nodes in the independent set.");
		#pragma omp parallel for num_threads(_num_threads) schedule(dynamic)
		for (uint i = 0; i < independent_set.size(); i++) {
			_contract(independent_set[i]);
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::getCHGraph(CHGraph& ch_graph)
{

}

#endif
