#ifndef _UNIT_TESTS_H
#define _UNIT_TESTS_H


#include "nodes_and_edges.h"
#include "graph.h"

#include <iostream>

#define Test(x)\
if (!(x)) {\
	std::cout << "TEST_FAILED: " << #x << std::endl;\
}\

namespace unit_tests
{
	void testNodesAndEdges()
	{
		typedef MetricEdge<Edge> MetricEdge;
		typedef CHEdge<MetricEdge> CHEdge;

		CHNode node0(Node(0), 0);
		CHNode node1(Node(1), 1);
		CHNode node2(Node(2), 2);
		CHNode node3(Node(3), 3);

		MetricEdge edge0(Edge(0, node0.id, node1.id, 42), 23);
		MetricEdge edge1(Edge(1, node2.id, node3.id, 24), 32);
		CHEdge ch_edge(MetricEdge(
					Edge(2, node0.id, node3.id, 66), 55), edge0.id, edge1.id);

		Test(edge0.otherNode(IN) == 0);
		Test(ch_edge.otherNode(OUT) == 3);
	}

	void testGraph()
	{
		Graph<Node, Edge> g;
		Test(g.read("../data/15kSZHK.txt"));

		g.initOffsets<EdgeSortSrc<Edge>, EdgeSortSrc<Edge> >();
		g.initIdToIndex();

		Graph<Node, Edge>::EdgeIt it(g, 0, OUT);
		Test(it.getNext().tgt == 6588);
		Test(it.getNext().tgt == 11947);
		Test(!it.hasNext());

		// TODO (Fast) vollständiger Test des Iterators.

		for (NodeID node_id(0); node_id<g.getNrOfNodes(); node_id++) {
			Test(g.getNode(node_id).id == node_id);
		}
		for (EdgeID edge_id(0); edge_id<g.getNrOfEdges(); edge_id++) {
			Test(g.getEdge(edge_id).id == edge_id);
		}
	}

	void testAll()
	{
		unit_tests::testNodesAndEdges();
		unit_tests::testGraph();
	}
}

#endif
