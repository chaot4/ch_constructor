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
		static bool readFormat1Node(Format1Node& node, std::stringstream& ss);
		static bool writeFormat1Node(Format1Node const& node, std::ofstream& os);

		/* read Edges */
		static bool readFormat1Edge(Format1Edge& edge, std::stringstream& ss);
		static bool writeFormat1Edge(Format1Edge const& edge, std::ofstream& os);

	public:
		/* Nodes and edges for a graph */
		struct Data;

		/* read Graphfiles */
		static bool readSTD(Data& data, std::string const& filename);
//		static bool writeSTD(Data const& data, std::string const& filename);

};

template <typename Node, typename Edge>
struct Parser<Node,Edge>::Data
{
	std::vector<Node> nodes;
	std::vector<Edge> edges;
};

/* Graphfiles */
template <typename Node, typename Edge>
bool Parser<Node,Edge>::readSTD(Data& data,std::string const& filename)
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
			readFormat1Node(parser_node, ss);
			data.nodes[i] = Node(parser_node);
		}
		std::sort(data.nodes.begin(), data.nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		data.edges.resize(nr_of_edges);

		for (uint i(0); i<nr_of_edges; i++) {
			Format1Edge parser_edge;
			readFormat1Edge(parser_edge, ss);
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

//template <typename Node, typename Edge>
//bool Parser<Node,Edge>::writeSTD(Data const& data, std::string const& filename)
//{
//	std::ofstream f(filename.c_str());
//
//	if (f.is_open()) {
//		uint nr_of_nodes(data.nodes.size());
//		uint nr_of_edges(data.edges.size());
//
//		f << nr_of_nodes << std::endl;
//		f << nr_of_edges << std::endl;
//
//		for (uint i(0); i<nr_of_nodes; i++) {
//			data.nodes[i].write(f);
//			f << std::endl;
//		}
//
//		for (uint i(0); i<nr_of_edges; i++) {
//			data.edges[i].write(f);
//			f << std::endl;
//		}
//
//		f.close();
//	}
//	else {
//		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" << filename
//			<< "\'. Exiting." << std::endl;
//		return false;
//	}
//
//	return true;
//}


/* Nodes */
template <typename Node, typename Edge>
bool Parser<Node,Edge>::readFormat1Node(Format1Node& node, std::stringstream& ss)
{
	ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;
	return true;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeFormat1Node(Format1Node const& node, std::ofstream& os)
{

	os << node.id << node.osm_id << node.lat << node.lon << node.elev;
	return true;
}

/* Edges */
template <typename Node, typename Edge>
bool Parser<Node,Edge>::readFormat1Edge(Format1Edge& edge, std::stringstream& ss)
{
	ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;
	return true;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeFormat1Edge(Format1Edge const& edge, std::ofstream& os)
{
	os << edge.src << edge.tgt << edge.dist << edge.type << edge.speed;
	return true;
}

}

#endif
