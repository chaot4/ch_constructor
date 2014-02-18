#ifndef _PARSER_H
#define _PARSER_H

#include "nodes_and_edges.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace chc
{

template <typename Node, typename Edge>
class Parser
{
	private:
		/* read Nodes */
		static Format1Node readFormat1Node(std::stringstream& ss);
		static bool writeFormat1Node(Format1Node const& node, std::ofstream& os);

		/* read Edges */
		static Format1Edge readFormat1Edge(std::stringstream& ss);
		static bool writeFormat1Edge(Format1Edge const& edge, std::ofstream& os);

	public:
		/* Nodes and edges of a graph */
		struct InData;
		struct OutData;

		/* read Graphfiles */
		static bool readSTD(InData& data, std::string const& filename);
		static bool writeSTD(OutData data, std::string const& filename);

};

template <typename Node, typename Edge>
struct Parser<Node,Edge>::InData
{
	std::vector<Node> nodes;
	std::vector<Edge> edges;
};

template <typename Node, typename Edge>
struct Parser<Node,Edge>::OutData
{
	std::vector<Node> const& nodes;
	std::vector<Edge> const& edges;

	OutData(std::vector<Node> const& nodes,
			std::vector<Edge> const& edges)
		: nodes(nodes), edges(edges) {}
};

/* Graphfiles */
template <typename Node, typename Edge>
bool Parser<Node,Edge>::readSTD(InData& data,std::string const& filename)
{
	std::ifstream f(filename.c_str());

	if (f.is_open()) {
		std::string file;

		/* Read the file into RAM */
		f.seekg(0, std::ios::end);
		file.reserve(f.tellg());
		f.seekg(0, std::ios::beg);
		file.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());

		uint nr_of_nodes;
		uint nr_of_edges;

		std::stringstream ss(file);
		ss >> nr_of_nodes >> nr_of_edges;
		Print("Number of nodes: " << nr_of_nodes);
		Print("Number of edges: " << nr_of_edges);

		/* Read the nodes. */
		data.nodes.resize(nr_of_nodes);
		for (uint i(0); i<nr_of_nodes; i++){
			Format1Node parser_node;
			parser_node = readFormat1Node(ss);
			data.nodes[i] = Node(parser_node);
		}
		std::sort(data.nodes.begin(), data.nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		data.edges.resize(nr_of_edges);

		for (uint i(0); i<nr_of_edges; i++) {
			Format1Edge parser_edge;
			parser_edge = readFormat1Edge(ss);
			data.edges[i] = Edge(parser_edge, i);
		}

		Print("Read all the edges.");

		f.close();
	}
	else {
		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" << filename
			<< "\'. Exiting." << std::endl;
		return false;
	}

	return true;
}

template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeSTD(OutData data, std::string const& filename)
{
	std::ofstream f(filename.c_str());

	if (f.is_open()) {
		uint nr_of_nodes(data.nodes.size());
		uint nr_of_edges(data.edges.size());

		f << nr_of_nodes << std::endl;
		f << nr_of_edges << std::endl;

		for (uint i(0); i<nr_of_nodes; i++) {
			Format1Node parser_node(data.nodes[i]);
			writeFormat1Node(parser_node, f);
		}

		for (uint i(0); i<nr_of_edges; i++) {
			Format1Edge parser_edge(data.edges[i]);
			writeFormat1Edge(parser_edge, f);
		}

		f.close();
	}
	else {
		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" << filename
			<< "\'. Exiting." << std::endl;
		return false;
	}

	return true;
}


/* Nodes */
template <typename Node, typename Edge>
Format1Node Parser<Node,Edge>::readFormat1Node(std::stringstream& ss)
{
	Format1Node node;
	ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;
	return node;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeFormat1Node(Format1Node const& node, std::ofstream& os)
{
	os << node.id << " " << node.osm_id << " " << node.lat << " " <<
		node.lon << " " << node.elev << std::endl;
	return true;
}

/* Edges */
template <typename Node, typename Edge>
Format1Edge Parser<Node,Edge>::readFormat1Edge(std::stringstream& ss)
{
	Format1Edge edge;
	ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;
	return edge;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeFormat1Edge(Format1Edge const& edge, std::ofstream& os)
{
	os << edge.src << " " << edge.tgt << " " << edge.dist << " " <<
		edge.type << " " << edge.speed << std::endl;
	return true;
}

}

#endif
