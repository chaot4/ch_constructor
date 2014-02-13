#ifndef _UNIT_TESTS_H
#define _UNIT_TESTS_H


#include "nodes_and_edges.h"
#include "graph.h"
#include "chgraph.h"
#include "ch_constructor.h"
#include "dijkstra.h"

#include <map>
#include <iostream>
#include <random>
#include <chrono>

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
		unit_tests::testCHDijkstra();
		unit_tests::testDijkstra();
	}
}

void unit_tests::testNodesAndEdges()
{
	Print("\n=================================");
	Print("TEST: Start Nodes And Edges test.");
	Print("=================================\n");

	typedef MetricEdge<Edge> MetricEdge;
	typedef CHEdge<MetricEdge> CHEdge;

	CHNode<Node> node0(Node(0), 0);
	CHNode<Node> node1(Node(1), 1);
	CHNode<Node> node2(Node(2), 2);

	MetricEdge edge0(Edge(0, node0.id, node1.id, 42), 23);
	MetricEdge edge1(Edge(1, node1.id, node2.id, 24), 32);
	CHEdge ch_edge(MetricEdge(
			Edge(2, node0.id, node2.id, 66), 55),
			node1.id, edge0.id, edge1.id);

	Test(edge0.otherNode(IN) == 0);
	Test(ch_edge.otherNode(OUT) == 2);

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
	g.init<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >("../data/15kSZHK.txt");

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

	typedef CHEdge<Edge> Shortcut;
	typedef SCGraph<Node, Edge> CHGraph;

	CHGraph g;
	g.init<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >("../data/test");

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

	chc._calcIndependentSet(remaining_nodes, independent_set);
	Print("Size of the independent set of all nodes: " << independent_set.size());

	for (auto it(independent_set.begin()); it != independent_set.end(); it++) {
		is_in_ind_set[*it] = true;
	}

	for (auto it(independent_set.begin()); it != independent_set.end(); it++) {
		typename CHGraph::EdgeIt edge_it(g, *it, OUT);

		while (edge_it.hasNext()) {
			Shortcut const& edge(edge_it.getNext());
			Test(!is_in_ind_set[edge.tgt]);
		}
	}

	/*
	 * Test the contraction.
	 */
	chc.contract(all_nodes);
	chc.getCHGraph();

	// Export
	g.write("../data/ch_test");

	Print("\n====================================");
	Print("TEST: CHConstructor test successful.");
	Print("====================================\n");
}

void unit_tests::testCHDijkstra()
{
	Print("\n============================");
	Print("TEST: Start CHDijkstra test.");
	Print("============================\n");

	typedef SCGraph<Node, Edge> CHGraph;

	/* Init normal graph */
	Graph<Node,Edge> g;
	g.init<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >("../data/test");

	/* Init CH graph */
	CHGraph chg;
	chg.init<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >("../data/test");

	/* Build CH */
	CHConstructor<Node, Edge> chc(chg, 1);
	std::list<NodeID> all_nodes;
	for (uint i(0); i<chg.getNrOfNodes(); i++) {
		all_nodes.push_back(i);
	}
	chc.quick_contract(all_nodes, 4);
	chc.contract(all_nodes);
	chc.getCHGraph();

	// Export
	chg.write("../data/ch_test");

	/* Random Dijkstras */
	Print("\nStarting random Dijkstras.");
	uint nr_of_dij(10000);
	Dijkstra<Node, Edge> dij(g);
	CHDijkstra<Node, Edge> chdij(chg);

	std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<uint> dist(0,g.getNrOfNodes()-1);
	auto rand_node = std::bind (dist, gen);
	std::vector<EdgeID> path;
	for (uint i(0); i<nr_of_dij; i++) {
		NodeID src = rand_node();
		NodeID tgt = rand_node();
		Debug("From " << src << " to " << tgt << ".");
		Test(dij.calcShopa(src,tgt,path) == chdij.calcShopa(src,tgt,path));
	}

	Print("\n=================================");
	Print("TEST: CHDijkstra test successful.");
	Print("=================================\n");
}

void unit_tests::testDijkstra()
{
	Print("\n============================");
	Print("TEST: Start Dijkstra test.");
	Print("============================\n");

	Graph<Node, Edge> g;
	g.init<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >("../data/test");

	Dijkstra<Node, Edge> dij(g);
	std::vector<EdgeID> path;
	NodeID tgt(g.getNrOfNodes() - 1);
	uint dist = dij.calcShopa(0, tgt, path);

	Print("Dist of Dijkstra from 0 to " << tgt << ": " << dist);
	Test(dist == 9);

	Print("Shortest path from 0 to " << tgt << ":");
	for (uint i(0); i<path.size(); i++) {
		Edge const& edge(g.getEdge(path[i]));
		Print("EdgeID: " << edge.id << ", src: " << edge.src << ", tgt: " << edge.tgt);
	}

	Print("Test if shortest paths are the same from both sides for the 'test' graph.");
	for (NodeID src(0); src<g.getNrOfNodes(); src++) {
		for (NodeID tgt(src); tgt<g.getNrOfNodes(); tgt++) {
			Test(dij.calcShopa(src, tgt, path) == dij.calcShopa(tgt, src, path));
		}
	}

	Print("\n=================================");
	Print("TEST: Dijkstra test successful.");
	Print("=================================\n");
}

#endif
