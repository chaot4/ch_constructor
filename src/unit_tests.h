#ifndef _UNIT_TESTS_H
#define _UNIT_TESTS_H


#include "nodes_and_edges.h"
#include "graph.h"
#include "ch_constructor.h"

#include <map>
#include <iostream>

#define Test(x)\
if (!(x)) {\
	std::cout << "TEST_FAILED: " << #x << std::endl;\
	return;\
}\

namespace unit_tests
{
	void testAll()
	{
		unit_tests::testNodesAndEdges();
		unit_tests::testGraph();
		unit_tests::testCHConstructor();
	}
}

void unit_tests::testNodesAndEdges()
{
	Print("\n=================================");
	Print("TEST: Start Nodes And Edges test.");
	Print("=================================\n");

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

	Print("\n======================================");
	Print("TEST: Nodes and edges test successful.");
	Print("======================================\n");
}

void unit_tests::testGraph()
{
	Print("\n=======================");
	Print("TEST: Start Graph test.");
	Print("=======================\n");

	Graph<Node, Edge> g;
	Test(g.read("../data/15kSZHK.txt"));

	g.initOffsets<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >();
	g.initIdToIndex();

	/* Test the normal iterator. */
	for (NodeID node_id(0); node_id<g.getNrOfNodes(); node_id++) {
		Test(g.getNode(node_id).id == node_id);

		/* Find for every out_edge the corresponding in edge. */
		Graph<Node, Edge>::EdgeIt it_out(g, node_id, OUT);
		while (it_out.hasNext()) {
			bool found(false);
			Edge const& out_edge(it_out.getNext());

			Graph<Node, Edge>::EdgeIt it_in(g, out_edge.tgt, IN);
			while (it_in.hasNext()) {
				Edge const& in_edge(it_in.getNext());
				if (in_edge.id == out_edge.id) {
					found = true;
				}
			}

			Test(found);
		}
	}

	for (EdgeID edge_id(0); edge_id<g.getNrOfEdges(); edge_id++) {
		Test(g.getEdge(edge_id).id == edge_id);
	}

	/* Test the offset iterator. */
	g.setEdgeSrcTgtToOffset();
	std::map<NodeID, NodeID> out_to_in_id;
	for (NodeID node_id(0); node_id<g.getNrOfNodes(); node_id++) {
		out_to_in_id.insert(
				std::pair<NodeID, NodeID>(
				g.getOffId(node_id, OUT), g.getOffId(node_id, IN)));
	}

	for (NodeID node_id(0); node_id<g.getNrOfNodes(); node_id++) {
		Test(g.getNode(node_id).id == node_id);

		/* Find for every out_edge the corresponding in edge. */
		Graph<Node, Edge>::OffEdgeIt it_out(g, g.getOffId(node_id, OUT), OUT);
		while (it_out.hasNext()) {
			bool found(false);
			Edge const& out_edge(it_out.getNext());

			Graph<Node, Edge>::OffEdgeIt it_in(
					g, out_to_in_id[out_edge.tgt], IN);
			while (it_in.hasNext()) {
				Edge const& in_edge(it_in.getNext());
				if (in_edge.id == out_edge.id) {
					found = true;
				}
			}

			Test(found);
		}
	}

	Print("\n============================");
	Print("TEST: Graph test successful.");
	Print("============================\n");
}

void unit_tests::testCHConstructor()
{
	Print("\n===============================");
	Print("TEST: Start CHConstructor test.");
	Print("===============================\n");

	Graph<Node, Edge> g;
	g.read("../data/15kSZHK.txt");
	g.initOffsets<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >();
	g.initIdToIndex();

	CHConstructor<Node, Edge> chc(g, 2);

	/* 
	 * Test the independent set construction.
	 */

	std::list<NodeID> all_nodes;
	for (uint i(0); i<g.getNrOfNodes(); i++) {
		all_nodes.push_back(i);
	}

	std::list<NodeID> remaining_nodes(all_nodes);
	std::vector<NodeID> independent_set;
	std::vector<bool> is_in_ind_set(g.getNrOfNodes(), false);

	chc._extractIndependentSet(remaining_nodes, independent_set);
	Print("Size of the independent set of all nodes: " << independent_set.size());

	for (auto it(independent_set.begin()); it != independent_set.end(); it++) {
		is_in_ind_set[*it] = true;
	}

	for (auto it(independent_set.begin()); it != independent_set.end(); it++) {
		typename Graph<Node, Edge>::EdgeIt edge_it(g, *it, OUT);

		while (edge_it.hasNext()) {
			Edge const& edge(edge_it.getNext());
			Test(!is_in_ind_set[edge.tgt]);
		}
	}

	/*
	 * Test the contraction.
	 */
	chc.contract(all_nodes);

	Print("\n====================================");
	Print("TEST: CHConstructor test successful.");
	Print("====================================\n");
}

#endif
