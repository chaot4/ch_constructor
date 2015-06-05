/* always Print() stuff */
#ifdef NVERBOSE
# undef NVERBOSE
#endif

#include "unit_tests.h"

#include "nodes_and_edges.h"
#include "graph.h"
#include "file_formats.h"
#include "chgraph.h"
#include "ch_constructor.h"
#include "dijkstra.h"
#include "prioritizers.h"

#include <map>
#include <iostream>
#include <random>
#include <chrono>

namespace chc
{

void unit_tests::testAll()
{
	unit_tests::testNodesAndEdges();
	unit_tests::testGraph();
	unit_tests::testCHConstructor();
	unit_tests::testCHDijkstra();
	unit_tests::testDijkstra();
	unit_tests::testPrioritizers();
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
	CHEdge ch_edge(make_shortcut(edge0, edge1));

	Test(otherNode(edge0, EdgeType::IN) == 0);
	Test(otherNode(ch_edge, EdgeType::OUT) == 2);

	Print("\n======================================");
	Print("TEST: Nodes and edges test successful.");
	Print("======================================\n");
}

void unit_tests::testGraph()
{
	Print("\n=======================");
	Print("TEST: Start Graph test.");
	Print("=======================\n");

	Graph<OSMNode, Edge> g;
	g.init(FormatSTD::Reader::readGraph<OSMNode, Edge>("../test_data/15kSZHK.txt"));

	/* Test the normal iterator. */
	for (NodeID node_id(0); node_id<g.getNrOfNodes(); node_id++) {
		Test(g.getNode(node_id).id == node_id);

		/* Find for every out_edge the corresponding in edge. */
		for (auto const& out_edge: g.nodeEdges(node_id, EdgeType::OUT)) {
			bool found(false);

			for (auto const& in_edge: g.nodeEdges(out_edge.tgt, EdgeType::IN)) {
				if (equalEndpoints(in_edge, out_edge)) {
					found = true;
					break;
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

	typedef CHEdge<OSMEdge> Shortcut;
	typedef CHGraph<OSMNode, OSMEdge> CHGraphOSM;

	CHGraphOSM g;
	g.init(FormatSTD::Reader::readGraph<OSMNode, Shortcut>("../test_data/test"));

	CHConstructor<OSMNode, OSMEdge> chc(g, 2);

	/*
	 * Test the independent set construction.
	 */

	std::vector<NodeID> all_nodes(g.getNrOfNodes());
	for (NodeID i(0); i<all_nodes.size(); i++) {
		all_nodes[i] = i;
	}

	std::vector<NodeID> remaining_nodes(all_nodes);
	std::vector<bool> is_in_ind_set(g.getNrOfNodes(), false);

	auto independent_set = chc.calcIndependentSet(remaining_nodes);
	Print("Size of the independent set of all nodes: " << independent_set.size());

	for (NodeID node: independent_set) {
		is_in_ind_set[node] = true;
	}

	for (NodeID node: independent_set) {
		for (auto const& edge: g.nodeEdges(node, EdgeType::OUT)) {
			Test(!is_in_ind_set[edge.tgt]);
		}
	}

	/*
	 * Test the contraction.
	 */
	chc.contract(all_nodes);

	// Export
	writeCHGraphFile<FormatSTD::Writer>("../out/ch_test", g.exportData());

	Print("\n====================================");
	Print("TEST: CHConstructor test successful.");
	Print("====================================\n");
}

void unit_tests::testCHDijkstra()
{
	Print("\n============================");
	Print("TEST: Start CHDijkstra test.");
	Print("============================\n");

	typedef CHEdge<OSMEdge> Shortcut;
	typedef CHGraph<OSMNode, OSMEdge> CHGraphOSM;

	/* Init normal graph */
	Graph<OSMNode, OSMEdge> g;
	g.init(FormatSTD::Reader::readGraph<OSMNode, OSMEdge>("../test_data/15kSZHK.txt"));

	/* Init CH graph */
	CHGraphOSM chg;
	chg.init(FormatSTD::Reader::readGraph<OSMNode, Shortcut>("../test_data/15kSZHK.txt"));

	/* Build CH */
	CHConstructor<OSMNode, OSMEdge> chc(chg, 2);
	std::vector<NodeID> all_nodes(g.getNrOfNodes());
	for (NodeID i(0); i<all_nodes.size(); i++) {
		all_nodes[i] = i;
	}
	chc.quickContract(all_nodes, 4, 5);
	chc.contract(all_nodes);
	chc.rebuildCompleteGraph();

	/* Random Dijkstras */
	Print("\nStarting random Dijkstras.");
	uint nr_of_dij(10);
	Dijkstra<OSMNode, OSMEdge> dij(g);
	CHDijkstra<OSMNode, OSMEdge> chdij(chg);

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

	// Export (destroys graph data)
	writeCHGraphFile<FormatSTD::Writer>("../out/ch_15kSZHK.txt", chg.exportData());

	Print("\n=================================");
	Print("TEST: CHDijkstra test successful.");
	Print("=================================\n");
}

void unit_tests::testDijkstra()
{
	Print("\n============================");
	Print("TEST: Start Dijkstra test.");
	Print("============================\n");

	Graph<OSMNode, Edge> g;
	g.init(FormatSTD::Reader::readGraph<OSMNode, Edge>("../test_data/test"));

	Dijkstra<OSMNode, Edge> dij(g);
	std::vector<EdgeID> path;
	NodeID tgt(g.getNrOfNodes() - 1);
	uint dist = dij.calcShopa(0, tgt, path);

	Print("Dist of Dijkstra from 0 to " << tgt << ": " << dist);
	Test(dist == 18);

	Print("Shortest path from 0 to " << tgt << ":");
	for (auto edge_id: path) {
		Edge const& edge(g.getEdge(edge_id));
		Print("EdgeID: " << edge_id << ", src: " << edge.src << ", tgt: " << edge.tgt);
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

void unit_tests::testPrioritizers()
{
	Print("\n=============================");
	Print("TEST: Start Prioritizer test.");
	Print("=============================\n");

	typedef CHEdge<OSMEdge> Shortcut;
	typedef CHGraph<OSMNode, OSMEdge> CHGraphOSM;

	/* Init normal graph */
	Graph<OSMNode, OSMEdge> g;
	g.init(FormatSTD::Reader::readGraph<OSMNode, OSMEdge>("../test_data/test"));

	size_t const last = from_enum(LastPrioritizerType);
	for (size_t t = 0; t <= last; ++t) {
		PrioritizerType type(static_cast<PrioritizerType>(t));
		Print("\n------------------------------------");
		Print("Testing prioritizer type: " << to_string(type));
		Print("------------------------------------\n");

		/* Init CH graph */
		CHGraphOSM chg;
		chg.init(FormatSTD::Reader::readGraph<OSMNode, Shortcut>("../test_data/test"));

		/* Build CH */
		CHConstructor<OSMNode, OSMEdge> chc(chg, 2);
		std::vector<NodeID> all_nodes(g.getNrOfNodes());
		for (NodeID i(0); i<all_nodes.size(); i++) {
			all_nodes[i] = i;
		}
		std::random_shuffle(all_nodes.begin(), all_nodes.end()); /* random initial node order */
		auto prioritizer(createPrioritizer(type, chg, chc));
		if (prioritizer == nullptr && type == PrioritizerType::NONE) { continue; }
		chc.contract(all_nodes, *prioritizer);
		chc.rebuildCompleteGraph();

		/* Random Dijkstras */
		Print("\nStarting random Dijkstras.");
		uint nr_of_dij(1000);
		Dijkstra<OSMNode, OSMEdge> dij(g);
		CHDijkstra<OSMNode, OSMEdge> chdij(chg);

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
	}

	Print("\n==================================");
	Print("TEST: Prioritizer test successful.");
	Print("==================================\n");
}

}
