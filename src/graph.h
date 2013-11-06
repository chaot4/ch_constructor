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

namespace unit_tests
{
	void testGraph();
}

template <typename Node, typename Edge>
class Graph
{
	protected:
		std::vector<Node> _nodes;

		std::vector<uint> _out_offsets;
		std::vector<uint> _in_offsets;
		std::vector<Edge> _out_edges;
		std::vector<Edge> _in_edges;

		/* Maps edge id to index in the _out_edge vector. */
		std::vector<EdgeID> _id_to_index;

	public:
		class EdgeIt;
		class OffEdgeIt;

		/* Read the nodes and edges from the file <filename>.
		 * The offset vectors aren't initialized! */
		bool read(std::string const& filename);

		template <class InEdgeSort>
		void sortInEdges();
		template <class OutEdgeSort>
		void sortOutEdges();

		void initOffsets();
		void initIdToIndex();
		void setEdgeSrcTgtToOffset();

		uint getNrOfNodes() const;
		uint getNrOfEdges() const;
		uint getNrOfEdges(NodeID node_id, EdgeType type) const;
		Edge const& getEdge(EdgeID edge_id);
		Node const& getNode(NodeID node_id);
		NodeID getOffId(NodeID node_id, EdgeType type);

		friend void unit_tests::testGraph();
};

/*
 * Graph member functions.
 */

template <typename Node, typename Edge>
bool Graph<Node, Edge>::read(std::string const& filename)
{
	std::ifstream f(filename.c_str());

	if (f.is_open()) {
		uint nr_of_nodes(_nodes.size());
		uint nr_of_edges(_out_edges.size());
		std::string file;

		/* Read the file into RAM */
		f.seekg(0, std::ios::end);
		file.reserve(f.tellg());
		f.seekg(0, std::ios::beg);
		file.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());

		std::stringstream ss(file);

		ss >> nr_of_nodes >> nr_of_edges;
		Print("Number of nodes: " << nr_of_nodes);
		Print("Number of edges: " << nr_of_edges);

		/* Read the nodes. */
		_nodes.resize(nr_of_nodes);
		for (uint i(0); i<nr_of_nodes; i++){
			_nodes[i].read(ss);
		}
		std::sort(_nodes.begin(), _nodes.end());

		Print("Read all the nodes.");

		/* Read the edges into _out_edges and _in_edges. */
		_out_edges.resize(nr_of_edges);
		_in_edges.reserve(nr_of_edges);

		for (EdgeID edge_id(0); edge_id<nr_of_edges; edge_id++) {
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
template <class InEdgeSort>
void Graph<Node, Edge>::sortInEdges()
{
	Print("Sort the incomming edges.");

	std::sort(_in_edges.begin(), _in_edges.end(), InEdgeSort());
	assert(std::is_sorted(_in_edges.begin(), _in_edges.end(), InEdgeSort()));
}

template <typename Node, typename Edge>
template <class OutEdgeSort>
void Graph<Node, Edge>::sortOutEdges()
{
	Print("Sort the outgoing edges.");

	std::sort(_out_edges.begin(), _out_edges.end(), OutEdgeSort());
	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), OutEdgeSort()));
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::initOffsets()
{
	Print("Init the offsets.");

	uint nr_of_nodes(_nodes.size());
	uint nr_of_edges(_out_edges.size());

	std::vector<uint> out_edge_count(nr_of_nodes, 0);
	std::vector<uint> in_edge_count(nr_of_nodes, 0);
	for (uint i(0); i<nr_of_edges; i++) {
		out_edge_count[_out_edges[i].src]++;
		in_edge_count[_in_edges[i].tgt]++;
	}

	uint out_sum(0);
	uint in_sum(0);
	_out_offsets.resize(nr_of_nodes + 1);
	_in_offsets.resize(nr_of_nodes + 1);
	for (NodeID i(0); i<nr_of_nodes; i++) {
		_out_offsets[i] = out_sum;
		_in_offsets[i] = in_sum;
		out_sum += out_edge_count[i];
		in_sum += in_edge_count[i];
	}
	assert(out_sum == nr_of_edges);
	assert(in_sum == nr_of_edges);
	_out_offsets[nr_of_nodes] = nr_of_edges;
	_in_offsets[nr_of_nodes] = nr_of_edges;
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::initIdToIndex()
{
	Print("Renew the index mapper.");

	_id_to_index.resize(_out_edges.size());
	for (uint i(0); i<_out_edges.size(); i++) {
		_id_to_index[_out_edges[i].id] = i;
	}
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::setEdgeSrcTgtToOffset()
{
	assert(_in_edges.size() == _out_edges.size());

	for (uint i(0); i<_in_edges.size(); i++) {
		_in_edges[i].src = _in_offsets[_in_edges[i].src];
		_in_edges[i].tgt = _out_offsets[_in_edges[i].tgt];
		_out_edges[i].src = _in_offsets[_out_edges[i].src];
		_out_edges[i].tgt = _out_offsets[_out_edges[i].tgt];
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
NodeID Graph<Node, Edge>::getOffId(NodeID node_id, EdgeType type)
{
	if (type == OUT) {
		return _out_offsets[node_id];
	}
	else {
		return _in_offsets[node_id];
	}
}

template <typename Node, typename Edge>
uint Graph<Node, Edge>::getNrOfNodes() const
{
	return _nodes.size();
}

template <typename Node, typename Edge>
uint Graph<Node, Edge>::getNrOfEdges() const
{
	return _out_edges.size();
}

template <typename Node, typename Edge>
uint Graph<Node, Edge>::getNrOfEdges(NodeID node_id, EdgeType type) const
{
	if (type == IN) {
		return _in_offsets[node_id+1] - _in_offsets[node_id];
	}
	else {
		return _out_offsets[node_id+1] - _out_offsets[node_id];
	}
}

/*
 * EdgeIt.
 */

template <typename Node, typename Edge>
class Graph<Node, Edge>::EdgeIt
{
	private:
		Edge const* _current;
		Edge const* _end;
	public:
		EdgeIt(Graph<Node, Edge> const& g, NodeID node_id, EdgeType type);

		bool hasNext() const;
		Edge const& getNext();
};

template <typename Node, typename Edge>
Graph<Node, Edge>::EdgeIt::EdgeIt(Graph<Node, Edge> const& g, NodeID node_id, EdgeType type)
{
	if (type == OUT) {
		_current = &g._out_edges[g._out_offsets[node_id]];
		_end = &g._out_edges[g._out_offsets[node_id+1]];
	}
	else {
		_current = &g._in_edges[g._in_offsets[node_id]];
		_end = &g._in_edges[g._in_offsets[node_id+1]];
	}
}

template <typename Node, typename Edge>
bool Graph<Node, Edge>::EdgeIt::hasNext() const
{
	return _current != _end;
}

template <typename Node, typename Edge>
Edge const& Graph<Node, Edge>::EdgeIt::getNext()
{
	return *(_current++);
}

/*
 * OffEdgeIt
 */

template <typename Node, typename Edge>
class Graph<Node, Edge>::OffEdgeIt
{
	private:
		EdgeType _type;
		NodeID _off_node_id;
		Edge const* _current;
		Edge const* _end_of_vector;
	public:
		OffEdgeIt(Graph<Node, Edge> const& g, NodeID off_node_id, EdgeType type);

		bool hasNext() const;
		Edge const& getNext();
};

template <typename Node, typename Edge>
Graph<Node, Edge>::OffEdgeIt::OffEdgeIt(Graph<Node, Edge> const& g,
		NodeID off_node_id, EdgeType type)
	: _type(type), _off_node_id(off_node_id)
{
	if (type == OUT) {
		_current = &g._out_edges[off_node_id];
		_end_of_vector = &*g._out_edges.end();
	}
	else {
		_current = &g._in_edges[off_node_id];
		_end_of_vector = &*g._in_edges.end();
	}
}

template <typename Node, typename Edge>
bool Graph<Node, Edge>::OffEdgeIt::hasNext() const
{
	NodeID next_off_node_id;

	if (_current != _end_of_vector) {
		next_off_node_id = _current->otherNode(_type);
	}
	else {
		return false;
	}

	if (next_off_node_id == _off_node_id) {
		return true;
	}
	else {
		return false;
	}
}

template <typename Node, typename Edge>
Edge const& Graph<Node, Edge>::OffEdgeIt::getNext()
{
	return *(_current++);
}

#endif
