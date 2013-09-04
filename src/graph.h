#ifndef _GRAPH_H
#define _GRAPH_H

#include "defs.h"
#include "nodes_and_edges.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>

template <typename Node, typename Edge>
class Graph
{
	private:
		uint _nr_of_nodes;
		uint _nr_of_edges;

		std::vector<Node> _nodes;

		std::vector<uint> _out_offsets;
		std::vector<uint> _in_offsets;
		std::vector<Edge> _out_edges;
		std::vector<Edge> _in_edges;

		/* The index is of the _out_edges vector. */
		std::vector<EdgeID> _id_to_index;

	public:
		class EdgeIt;

		/* Read the nodes and edges from the file <filename>.
		 * The offset vectors aren't initialized! */
		bool read(std::string const& filename);

		/* Inits in and out offsets for the in and out edge vectors.
		 * The edges are primarily sorted by src id (out) or tgt id (in).
		 * Secondarily they are sorted according to <edge_sort> */
		template <class OutEdgeSort, class InEdgeSort>
		void initOffsets();
		void initIdToIndex();

		uint getNrOfNodes();
		uint getNrOfEdges();
		Edge const& getEdge(EdgeID edge_id);
		Node const& getNode(NodeID node_id);
};

/*
 * Graph member functions.
 */

template <typename Node, typename Edge>
bool Graph<Node, Edge>::read(std::string const& filename)
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

		ss >> _nr_of_nodes >> _nr_of_edges;
		Print("Number of nodes: " << _nr_of_nodes);
		Print("Number of edges: " << _nr_of_edges);

		/* Read the nodes. */
		_nodes.resize(_nr_of_nodes);
		for (uint i(0); i<_nr_of_nodes; i++){
			_nodes[i].read(ss);
		}
		std::sort(_nodes.begin(), _nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		_out_edges.resize(_nr_of_edges);
		_in_edges.reserve(_nr_of_edges);

		for (EdgeID edge_id(0); edge_id<_nr_of_edges; edge_id++) {
			Edge& edge(_out_edges[edge_id]);
			edge.id = edge_id;
			edge.read(ss);

			_in_edges.push_back(edge);
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
template <class OutEdgeSort, class InEdgeSort>
void Graph<Node, Edge>::initOffsets()
{
	std::sort(_out_edges.begin(), _out_edges.end(), OutEdgeSort());
	std::sort(_in_edges.begin(), _in_edges.end(), InEdgeSort());
	assert(std::is_sorted(_out_edges.begin(), _out_edges.end()));
	assert(std::is_sorted(_in_edges.begin(), _in_edges.end()));

	std::vector<uint> out_edge_count(_nr_of_nodes, 0);
	std::vector<uint> in_edge_count(_nr_of_nodes, 0);
	for (uint i(0); i<_nr_of_edges; i++) {
		out_edge_count[_out_edges[i].src]++;
		in_edge_count[_in_edges[i].tgt]++;
	}

	uint out_sum(0);
	uint in_sum(0);
	_out_offsets.resize(_nr_of_nodes + 1);
	_in_offsets.resize(_nr_of_nodes + 1);
	for (NodeID i(0); i<_nr_of_nodes; i++) {
		_out_offsets[i] = out_sum;
		_in_offsets[i] = in_sum;
		out_sum += out_edge_count[i];
		in_sum += in_edge_count[i];
	}
	assert(out_sum == _nr_of_edges);
	assert(in_sum == _nr_of_edges);
	_out_offsets[_nr_of_nodes] = _nr_of_edges;
	_in_offsets[_nr_of_nodes] = _nr_of_edges;
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::initIdToIndex()
{
	_id_to_index.resize(_nr_of_edges);
	for (uint i(0); i<_out_edges.size(); i++) {
		_id_to_index[_out_edges[i].id] = i;
	}
}

template <typename Node, typename Edge>
Edge const& Graph<Node, Edge>::getEdge(EdgeID edge_id)
{
	return _out_edges[_id_to_index[edge_id]];
}

template <typename Node, typename Edge>
Node const& Graph<Node, Edge>::getNode(NodeID node_id)
{
	return _nodes[node_id];
}

template <typename Node, typename Edge>
uint Graph<Node, Edge>::getNrOfNodes()
{
	return _nr_of_nodes;
}

template <typename Node, typename Edge>
uint Graph<Node, Edge>::getNrOfEdges()
{
	return _nr_of_edges;
}

/*
 * EdgeIt.
 */

template <typename Node, typename Edge>
class Graph<Node, Edge>::EdgeIt
{
	private:
		Edge* current;
		Edge* end;
	public:
		EdgeIt(Graph<Node, Edge>& g, NodeID node_id, EdgeType type);

		bool hasNext() const;
		Edge const& getNext();
};

template <typename Node, typename Edge>
Graph<Node, Edge>::EdgeIt::EdgeIt(Graph<Node, Edge>& g, NodeID node_id, EdgeType type)
{
	if (type == OUT) {
		current = &g._out_edges[g._out_offsets[node_id]];
		end = &g._out_edges[g._out_offsets[node_id+1]];
	}
	else {
		current = &g._in_edges[g._in_offsets[node_id]];
		end = &g._in_edges[g._in_offsets[node_id+1]];
	}
}

template <typename Node, typename Edge>
bool Graph<Node, Edge>::EdgeIt::hasNext() const
{
	return current != end;
}

template <typename Node, typename Edge>
Edge const& Graph<Node, Edge>::EdgeIt::getNext()
{
	return *(current++);
}

#endif
