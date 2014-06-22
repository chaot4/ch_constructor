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

namespace Parser {
	/* Data required to construct or write out graphs. */
	/* Node read/writes */
	OSMNode readOSMNode(std::stringstream& ss);
	bool writeOSMNode(OSMNode const& node, std::ofstream& os);
	GeoNode readGeoNode(std::stringstream& ss);
	bool writeGeoNode(GeoNode const& node, std::ofstream& os);

	/* Edges read/writes */
	OSMEdge readOSMEdge(std::stringstream& ss);
	bool writeOSMEdge(OSMEdge const& edge, std::ofstream& os);
	Edge readEdge(std::stringstream& ss);
	bool writeEdge(Edge const& edge, std::ofstream& os);

	/* Graphfile read/writes */
	template <typename NodeT, typename EdgeT>
	bool readSTD(GraphInData<NodeT,EdgeT>& data, std::string const& filename);
	template <typename NodeT, typename EdgeT>
	bool writeSTD(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename);
	template <typename NodeT, typename EdgeT>
	bool readSIMPLE(GraphInData<NodeT,EdgeT>& data, std::string const& filename);
	template <typename NodeT, typename EdgeT>
	bool writeSIMPLE(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename);
	template <typename NodeT, typename EdgeT>
	bool readFMI(GraphInData<NodeT,EdgeT>& data, std::string const& filename);
	template <typename NodeT, typename EdgeT>
	bool writeFMI_CH(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename);

	template <typename NodeT, typename EdgeT>
	bool read(GraphInData<NodeT,EdgeT>& data, std::string const& filename,
			FileFormat format);

	template <typename NodeT, typename EdgeT>
	bool write(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename,
			FileFormat format);

	template <typename NodeT, typename EdgeT>
	bool read(GraphInData<NodeT,EdgeT>& data, std::string const& filename,
			FileFormat format)
	{
		switch (format) {
			case STD:
				return readSTD<NodeT,EdgeT>(data, filename);
			case SIMPLE:
				return readSIMPLE<NodeT,EdgeT>(data, filename);
			case FMI:
				return readFMI<NodeT,EdgeT>(data, filename);
			default:
				std::cerr << "Unknown fileformat: " << format
					<< std::endl;
				return false;
		}
	}

	template <typename NodeT, typename EdgeT>
	bool write(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename,
					FileFormat format)
	{
		switch (format) {
			case STD:
				return writeSTD<NodeT,EdgeT>(data, filename);
			case SIMPLE:
				return writeSIMPLE<NodeT,EdgeT>(data, filename);
			case FMI_CH:
				return writeFMI_CH<NodeT,EdgeT>(data, filename);
			default:
				std::cerr << "Unknown fileformat: " << format
					<< std::endl;
				return false;
		}
	}

	/* Graphfiles */
	template <typename NodeT, typename EdgeT>
	bool readSTD(GraphInData<NodeT,EdgeT>& data,std::string const& filename)
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
				data.nodes[i] = static_cast<NodeT>(readOSMNode(ss));
				data.nodes[i].id = i;
			}
			std::sort(data.nodes.begin(), data.nodes.end());

			Print("Read all the nodes.");

			/* Read the edges into _out_edges and _in_edges. */
			data.edges.resize(nr_of_edges);

			for (uint i(0); i<nr_of_edges; i++) {
				data.edges[i] = static_cast<EdgeT>(readOSMEdge(ss));
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

	template <typename NodeT, typename EdgeT>
	bool writeSTD(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename)
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
				writeOSMNode(static_cast<OSMNode>(data.nodes[i]), f);
			}

			Print("Exported all nodes.");

			for (uint i(0); i<nr_of_edges; i++) {
				writeOSMEdge(static_cast<OSMEdge>(data.edges[i]), f);
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

	template <typename NodeT, typename EdgeT>
	bool readSIMPLE(GraphInData<NodeT,EdgeT>& data,std::string const& filename)
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
				data.nodes[i] = static_cast<NodeT>(readGeoNode(ss));
				data.nodes[i].id = i;
			}
			std::sort(data.nodes.begin(), data.nodes.end());

			Print("Read all the nodes.");

			/* Read the edges into _out_edges and _in_edges. */
			data.edges.resize(nr_of_edges);

			for (uint i(0); i<nr_of_edges; i++) {
				data.edges[i] = static_cast<EdgeT>(readEdge(ss));
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

	template <typename NodeT, typename EdgeT>
	bool writeSIMPLE(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename)
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
				writeGeoNode(static_cast<GeoNode>(data.nodes[i]), f);
			}

			Print("Exported all nodes.");

			for (uint i(0); i<nr_of_edges; i++) {
				writeEdge(static_cast<Edge>(data.edges[i]), f);
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

	template <typename NodeT, typename EdgeT>
	bool readFMI(GraphInData<NodeT,EdgeT>& data, std::string const& filename)
	{
		std::ifstream f(filename.c_str());

		if (f.is_open()) {
			std::string file;

			/* Read the file into RAM */
			f.seekg(0, std::ios::end);
			file.reserve(f.tellg());
			f.seekg(0, std::ios::beg);
			file.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
			std::stringstream ss(file);

			char c;
			ss.get(c);
			// Note that this loop also reads the first character after
			// all the comments are over.
			while (c == '#') {
				ss.ignore(1024, '\n');
				ss.get(c);
			}

			uint nr_of_nodes;
			uint nr_of_edges;
			ss >> nr_of_nodes >> nr_of_edges;
			Print("Number of nodes: " << nr_of_nodes);
			Print("Number of edges: " << nr_of_edges);

			/* Read the nodes. */
			data.nodes.resize(nr_of_nodes);
			for (uint i(0); i<nr_of_nodes; i++){
				data.nodes[i] = static_cast<NodeT>(readOSMNode(ss));
				data.nodes[i].id = i;
			}
			std::sort(data.nodes.begin(), data.nodes.end());

			Print("Read all the nodes.");

			/* Read the edges into _out_edges and _in_edges. */
			data.edges.resize(nr_of_edges);

			for (uint i(0); i<nr_of_edges; i++) {
				data.edges[i] = static_cast<EdgeT>(readOSMEdge(ss));
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

		return false;
	}

	template <typename NodeT, typename EdgeT>
	bool writeFMI_CH(GraphCHOutData<NodeT,EdgeT> data, std::string const& filename)
	{
		std::ofstream f(filename.c_str());

		if (f.is_open()) {

			/* Generate random id */
			char id[33];
			for(int i = 0; i < 32; i++) {
			    sprintf(id + i, "%x", rand() % 16);
			}

			/* Write header */
			f << "# Id : " << id << std::endl;
			f << "# Timestamp : " << time(0) << std::endl;
			f << "# Type: maxspeed" << std::endl;
			f << "# Revision: 1" << std::endl;
			f << std::endl;

			/* Write graph data */
			uint nr_of_nodes(data.nodes.size());
			uint nr_of_edges(data.edges.size());

			Print("Exporting " << nr_of_nodes << " nodes and "
					<< nr_of_edges << " edges to " << filename);

			f << nr_of_nodes << std::endl;
			f << nr_of_edges << std::endl;

			for (uint i(0); i<nr_of_nodes; i++) {
				// TODO write FMI_CH node
				writeOSMNode(static_cast<OSMNode>(data.nodes[i]), f);
			}

			Print("Exported all nodes.");

			for (uint i(0); i<nr_of_edges; i++) {
				// TODO write FMI_CH edge
				writeOSMEdge(static_cast<OSMEdge>(data.edges[i]), f);
			}

			Print("Exported all edges.");

			f.close();
		}
		else {
			std::cerr << "FATAL_ERROR: Couldn't open graph file \'" <<
				filename << "\'. Exiting." << std::endl;
			exit(1);
		}

		return false;
	}
}

}

#endif
