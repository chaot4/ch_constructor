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
		/* Data required to construct or write out graphs. */
		template <typename Node, typename Edge>
		struct InData;
		template <typename Node, typename Edge>
		struct OutData;

		/* Filetype specific nodes. */
		struct STDNode;
		struct SIMPLENode;

		/* Filetype specific edges */
		struct STDEdge;
		struct SIMPLEEdge;

	private:
		/* Node read/writes */
		static STDNode readSTDNode(std::stringstream& ss);
		static bool writeSTDNode(STDNode const& node, std::ofstream& os);
		static SIMPLENode readSIMPLENode(std::stringstream& ss);
		static bool writeSIMPLENode(SIMPLENode const& node, std::ofstream& os);

		/* Node casts */
		static void cast(STDNode const& in_node, Node& out_node);
		static void cast(SIMPLENode const& in_node, Node& out_node);
		template <typename Node>
		static void cast(STDNode const& in_node, CHNode<Node>& out_node);
		template <typename Node>
		static void cast(SIMPLENode const& in_node, CHNode<Node>& out_node);
		static void cast(Node const& in_node, STDNode& out_node);
		static void cast(Node const& in_node, SIMPLENode& out_node);
//		template <typename Node>
//		static void cast(CHNode<Node> const& in_node, STDNode& out_node);
//		template <typename Node>
//		static void cast(CHNode<Node> const& in_node, SIMPLENode& out_node);

		/* Edges read/writes */
		static STDEdge readSTDEdge(std::stringstream& ss);
		static bool writeSTDEdge(STDEdge const& edge, std::ofstream& os);
		static SIMPLEEdge readSIMPLEEdge(std::stringstream& ss);
		static bool writeSIMPLEEdge(SIMPLEEdge const& edge, std::ofstream& os);

		/* Edge casts */
		static void cast(STDEdge const& in_edge, Edge& out_edge);
		static void cast(SIMPLEEdge const& in_edge, Edge& out_edge);
		template <typename Edge>
		static void cast(STDEdge const& in_edge, CHEdge<Edge>& out_edge);
		template <typename Edge>
		static void cast(SIMPLEEdge const& in_edge, CHEdge<Edge>& out_edge);
		static void cast(Edge const& in_edge, STDEdge& out_edge);
		static void cast(Edge const& in_edge, SIMPLEEdge& out_edge);
//		template <typename Edge>
//		static void cast(CHEdge<Edge> const& in_edge, STDEdge& out_edge);
//		template <typename Edge>
//		static void cast(CHEdge<Edge> const& in_edge, SIMPLEEdge& out_edge);

		/* Graphfile read/writes */
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

	OutData(std::vector<Node> const& nodes, std::vector<Edge> const& edges)
		: nodes(nodes), edges(edges) {}
};

struct Parser::STDNode
{
	NodeID id;
	uint osm_id;
	double lat;
	double lon;
	int elev;

	STDNode();
	STDNode(NodeID id, uint osm_id, double lat, double lon, int elev);
};

struct Parser::SIMPLENode
{
	double lat;
	double lon;
	int elev;

	SIMPLENode();
	SIMPLENode(double lat, double lon, int elev);
};

struct Parser::STDEdge
{
	NodeID src;
	NodeID tgt;
	uint dist;
	uint type;
	int speed;

	STDEdge();
	STDEdge(NodeID src, NodeID tgt, uint dist, uint type, int speed);
};

struct Parser::SIMPLEEdge
{
	NodeID src;
	NodeID tgt;
	uint dist;

	SIMPLEEdge();
	SIMPLEEdge(NodeID src, NodeID tgt, uint dist);
};

/*
 * Node casts.
 */

template <typename Node>
void Parser::cast(STDNode const& in_node, CHNode<Node>& out_node)
{
	Node node;
	cast(in_node, node);
	out_node = CHNode<Node>(node, c::NO_LVL);
}

template <typename Node>
void Parser::cast(SIMPLENode const& in_node, CHNode<Node>& out_node)
{
	Node node;
	cast(in_node, node);
	out_node = CHNode<Node>(node, c::NO_LVL);
}

/*
 * Edge casts.
 */

template <typename Edge>
void Parser::cast(STDEdge const& in_edge, CHEdge<Edge>& out_edge)
{
	Edge edge;
	cast(in_edge, edge);
	out_edge = CHEdge<Edge>(edge, c::NO_EID, c::NO_EID, c::NO_NID);
}

template <typename Edge>
void Parser::cast(SIMPLEEdge const& in_edge, CHEdge<Edge>& out_edge)
{
	Edge edge;
	cast(in_edge, edge);
	out_edge = CHEdge<Edge>(edge, c::NO_EID, c::NO_EID, c::NO_NID);
}

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
			cast(readSTDNode(ss), data.nodes[i]);
			data.nodes[i].id = i;
		}
		std::sort(data.nodes.begin(), data.nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		data.edges.resize(nr_of_edges);

		for (uint i(0); i<nr_of_edges; i++) {
			cast(readSTDEdge(ss), data.edges[i]);
			data.edges[i].id = i;
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
			STDNode parser_node;
			cast(data.nodes[i], parser_node);
			writeSTDNode(parser_node, f);
		}

		Print("Exported all nodes.");

		for (uint i(0); i<nr_of_edges; i++) {
			STDEdge parser_edge;
			cast(data.edges[i], parser_edge);
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
			cast(readSIMPLENode(ss), data.nodes[i]);
			data.nodes[i].id = i;
		}
		std::sort(data.nodes.begin(), data.nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		data.edges.resize(nr_of_edges);

		for (uint i(0); i<nr_of_edges; i++) {
			cast(readSIMPLEEdge(ss), data.edges[i]);
			data.edges[i].id = i;
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
			SIMPLENode parser_node;
			cast(data.nodes[i], parser_node);
			writeSIMPLENode(parser_node, f);
		}

		Print("Exported all nodes.");

		for (uint i(0); i<nr_of_edges; i++) {
			SIMPLEEdge parser_edge;
			cast(data.edges[i], parser_edge);
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
