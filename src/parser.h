#ifndef _PARSER_H
#define _PARSER_H

#include "nodes_and_edges.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>

namespace chc
{

enum FileFormat { STD, SIMPLE, FMI, FMI_CH };
FileFormat toFileFormat(std::string const& format);

class Parser
{
	public:
		/* Nodes and edges of a graph */
		template <typename Node, typename Edge>
		struct InData;
		template <typename Node, typename Edge>
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
		template <typename Node, typename Edge>
		static bool readSTD(InData<Node,Edge>& data, std::string const& filename);
		template <typename Node, typename Edge>
		static bool writeSTD(OutData<Node,Edge> data, std::string const& filename);
		template <typename Node, typename Edge>
		static bool readSIMPLE(InData<Node,Edge>& data, std::string const& filename);
		template <typename Node, typename Edge>
		static bool writeSIMPLE(OutData<Node,Edge> data, std::string const& filename);
		template <typename Node, typename Edge>
		static bool readFMI(InData<Node,Edge>& data, std::string const& filename);
		template <typename Node, typename Edge>
		static bool writeFMI_CH(OutData<Node,Edge> data, std::string const& filename);

	public:
		template <typename Node, typename Edge>
		static bool read(InData<Node,Edge>& data, std::string const& filename,
				FileFormat format);

		template <typename Node, typename Edge>
		static bool write(OutData<Node,Edge> data, std::string const& filename,
				FileFormat format);
};



template <typename Node, typename Edge>
struct Parser::InData
{
	std::vector<Node> nodes;
	std::vector<Edge> edges;
};

template <typename Node, typename Edge>
struct Parser::OutData
{
	std::vector<Node> const& nodes;
	std::vector<Edge> const& edges;

	OutData(std::vector<Node> const& nodes,
			std::vector<Edge> const& edges)
		: nodes(nodes), edges(edges) {}
};

template <typename Node, typename Edge>
bool Parser::read(InData<Node,Edge>& data, std::string const& filename,
		FileFormat format)
{
	switch (format) {
		case STD:
			return readSTD<Node,Edge>(data, filename);
		case SIMPLE:
			return readSIMPLE<Node,Edge>(data, filename);
		case FMI:
			return readFMI<Node,Edge>(data, filename);
		default:
			std::cerr << "Unknown fileformat: " << format
				<< std::endl;
			return false;
	}
}

template <typename Node, typename Edge>
bool Parser::write(OutData<Node,Edge> data, std::string const& filename,
				FileFormat format)
{
	switch (format) {
		case STD:
			return writeSTD<Node,Edge>(data, filename);
		case SIMPLE:
			return writeSIMPLE<Node,Edge>(data, filename);
		case FMI_CH:
			return writeFMI_CH<Node,Edge>(data, filename);
		default:
			std::cerr << "Unknown fileformat: " << format
				<< std::endl;
			return false;
	}
}

/* Graphfiles */
template <typename Node, typename Edge>
bool Parser::readSTD(InData<Node,Edge>& data,std::string const& filename)
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
		exit(1);
	}

	return true;
}

template <typename Node, typename Edge>
bool Parser::writeSTD(OutData<Node,Edge> data, std::string const& filename)
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
		exit(1);
	}

	return true;
}

template <typename Node, typename Edge>
bool Parser::readSIMPLE(InData<Node,Edge>& data,std::string const& filename)
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
bool Parser::writeSIMPLE(OutData<Node,Edge> data, std::string const& filename)
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
bool Parser::readFMI(InData<Node,Edge>& data, std::string const& filename)
{
	// TODO
	return false;
}

template <typename Node, typename Edge>
bool Parser::writeFMI_CH(OutData<Node,Edge> data, std::string const& filename)
{
	// TODO
	return false;
}

}

#endif
