#ifndef _CH_CONSTRUCTOR
#define _CH_CONSTRUCTOR

#include "nodes_and_edges.h"
#include "graph.h"

#include <vector>
#include <list>

namespace unit_tests
{
	void testCHConstructor();
}

template <typename Node, typename Edge>
class CHConstructor{
	private:
		Graph<Node, Edge> const& _base_graph;

		void _contract(NodeID node);

		void _extractIndependentSet(std::list<NodeID>& nodes,
				std::list<NodeID>& independent_set);
		void _markNeighbours(NodeID node, std::vector<bool>& marked);
	public:
		CHConstructor(Graph<Node, Edge> const& base_graph);

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

}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::_extractIndependentSet(std::list<NodeID>& nodes,
		std::list<NodeID>& independent_set)
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
CHConstructor<Node, Edge>::CHConstructor(Graph<Node, Edge> const& base_graph)
		:_base_graph(base_graph){}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::contract(std::list<NodeID> nodes)
{
	while (!nodes.empty()) {
		std::list<NodeID> independent_set;
		_extractIndependentSet(nodes, independent_set);

		for (auto it(independent_set.begin()); it != independent_set.end(); it++) {
			_contract(*it);
		}
	}
}

template <typename Node, typename Edge>
void CHConstructor<Node, Edge>::getCHGraph(CHGraph& ch_graph)
{

}

#endif
