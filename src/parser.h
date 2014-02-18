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

enum FileFormat { STD = 0, SIMPLE = 1, FMI = 2 };

template <typename Node, typename Edge>
class Parser
{
	public:
		/* Nodes and edges of a graph */
		struct InData;
		struct OutData;

	private:
		/* Nodes */
		static STDNode readSTDNode(std::stringstream& ss);
		static bool writeSTDNode(STDNode const& node, std::ofstream& os);
		static SIMPLENode readSIMPLENode(std::stringstream& ss);
		static bool writeSIMPLENode(SIMPLENode const& node, std::ofstream& os);

		/* Edges */
		static STDEdge readSTDEdge(std::stringstream& ss);
		static bool writeSTDEdge(STDEdge const& edge, std::ofstream& os);
		static SIMPLEEdge readSIMPLEEdge(std::stringstream& ss);
		static bool writeSIMPLEEdge(SIMPLEEdge const& edge, std::ofstream& os);

		/* Graphfiles */
		static bool readSTD(InData& data, std::string const& filename);
		static bool writeSTD(OutData data, std::string const& filename);
		static bool readSIMPLE(InData& data, std::string const& filename);
		static bool writeSIMPLE(OutData data, std::string const& filename);
		static bool readFMI(InData& data, std::string const& filename);
		static bool writeFMI(OutData data, std::string const& filename);

	public:
		static bool read(InData& data, std::string const& filename,
				FileFormat format);
		static bool write(OutData data, std::string const& filename,
				FileFormat format);
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

template <typename Node, typename Edge>
bool Parser<Node,Edge>::read(InData& data, std::string const& filename,
		FileFormat format)
{
	switch (format) {
		case STD:
			return readSTD(data, filename);
		case SIMPLE:
			return readSIMPLE(data, filename);
		case FMI:
			return readFMI(data, filename);
		default:
			std::cerr << "Unknown fileformat: " << format
				<< std::endl;
			return false;
	}
}

template <typename Node, typename Edge>
bool Parser<Node,Edge>::write(OutData data, std::string const& filename,
				FileFormat format)
{
	switch (format) {
		case STD:
			return writeSTD(data, filename);
		case SIMPLE:
			return writeSIMPLE(data, filename);
		case FMI:
			return writeFMI(data, filename);
		default:
			std::cerr << "Unknown fileformat: " << format
				<< std::endl;
			return false;
	}
}

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
			STDNode parser_node;
			parser_node = readSTDNode(ss);
			data.nodes[i] = Node(parser_node);
		}
		std::sort(data.nodes.begin(), data.nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		data.edges.resize(nr_of_edges);

		for (uint i(0); i<nr_of_edges; i++) {
			STDEdge parser_edge;
			parser_edge = readSTDEdge(ss);
			data.edges[i] = Edge(parser_edge, i);
		}

		Print("Read all the edges.");

		f.close();
	}
	else {
		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" <<
			filename << "\'. Exiting." << std::endl;
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

		Print("Exporting " << nr_of_nodes << " nodes and "
				<< nr_of_edges << " edges to " << filename);

		f << nr_of_nodes << std::endl;
		f << nr_of_edges << std::endl;

		for (uint i(0); i<nr_of_nodes; i++) {
			STDNode parser_node(data.nodes[i]);
			writeSTDNode(parser_node, f);
		}

		Print("Exported all nodes.");

		for (uint i(0); i<nr_of_edges; i++) {
			STDEdge parser_edge(data.edges[i]);
			writeSTDEdge(parser_edge, f);
		}

		Print("Exported all edges.");

		f.close();
	}
	else {
		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" <<
			filename << "\'. Exiting." << std::endl;
		return false;
	}

	return true;
}

template <typename Node, typename Edge>
bool Parser<Node,Edge>::readSIMPLE(InData& data,std::string const& filename)
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
			SIMPLENode parser_node;
			parser_node = readSIMPLENode(ss);
			data.nodes[i] = Node(parser_node, i);
		}
		std::sort(data.nodes.begin(), data.nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		data.edges.resize(nr_of_edges);

		for (uint i(0); i<nr_of_edges; i++) {
			SIMPLEEdge parser_edge;
			parser_edge = readSIMPLEEdge(ss);
			data.edges[i] = Edge(parser_edge, i);
		}

		Print("Read all the edges.");

		f.close();
	}
	else {
		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" <<
			filename << "\'. Exiting." << std::endl;
		return false;
	}

	return true;
}

template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeSIMPLE(OutData data, std::string const& filename)
{
	std::ofstream f(filename.c_str());

	if (f.is_open()) {
		uint nr_of_nodes(data.nodes.size());
		uint nr_of_edges(data.edges.size());

		Print("Exporting " << nr_of_nodes << " nodes and "
				<< nr_of_edges << " edges to " << filename);

		f << nr_of_nodes << std::endl;
		f << nr_of_edges << std::endl;

		for (uint i(0); i<nr_of_nodes; i++) {
			SIMPLENode parser_node(data.nodes[i]);
			writeSIMPLENode(parser_node, f);
		}

		Print("Exported all nodes.");

		for (uint i(0); i<nr_of_edges; i++) {
			SIMPLEEdge parser_edge(data.edges[i]);
			writeSIMPLEEdge(parser_edge, f);
		}

		Print("Exported all edges.");

		f.close();
	}
	else {
		std::cerr << "FATAL_ERROR: Couldn't open graph file \'" <<
			filename << "\'. Exiting." << std::endl;
		return false;
	}

	return true;
}

template <typename Node, typename Edge>
bool Parser<Node,Edge>::readFMI(InData& data, std::string const& filename)
{
	// TODO
	return false;
}

template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeFMI(OutData data, std::string const& filename)
{
	// TODO
	return false;
}

/* Nodes */
template <typename Node, typename Edge>
STDNode Parser<Node,Edge>::readSTDNode(std::stringstream& ss)
{
	STDNode node;
	ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;
	return node;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeSTDNode(STDNode const& node, std::ofstream& os)
{
	os << node.id << " " << node.osm_id << " " << node.lat << " " <<
		node.lon << " " << node.elev << std::endl;
	return true;
}

template <typename Node, typename Edge>
SIMPLENode Parser<Node,Edge>::readSIMPLENode(std::stringstream& ss)
{
	SIMPLENode node;
	ss >> node.lat >> node.lon >> node.elev;
	return node;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeSIMPLENode(SIMPLENode const& node, std::ofstream& os)
{
	os << node.lat << " " << node.lon << " " << node.elev << std::endl;
	return true;
}

/* Edges */
template <typename Node, typename Edge>
STDEdge Parser<Node,Edge>::readSTDEdge(std::stringstream& ss)
{
	STDEdge edge;
	ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;
	return edge;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeSTDEdge(STDEdge const& edge, std::ofstream& os)
{
	os << edge.src << " " << edge.tgt << " " << edge.dist << " " <<
		edge.type << " " << edge.speed << std::endl;
	return true;
}

template <typename Node, typename Edge>
SIMPLEEdge Parser<Node,Edge>::readSIMPLEEdge(std::stringstream& ss)
{
	SIMPLEEdge edge;
	ss >> edge.src >> edge.tgt >> edge.dist;
	return edge;
}
template <typename Node, typename Edge>
bool Parser<Node,Edge>::writeSIMPLEEdge(SIMPLEEdge const& edge, std::ofstream& os)
{
	os << edge.src << " " << edge.tgt << " " << edge.dist << std::endl;
	return true;
}

}

#endif
