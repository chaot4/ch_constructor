#ifndef _GRAPH_H
#define _GRAPH_H

#include "defs.h"
#include "nodes_and_edges.h"
#include "parser.h"

#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

namespace chc
{

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
		std::vector<uint> _id_to_index;

		EdgeID _next_id;

		template <class InEdgeSort>
		void sortInEdges();
		template <class OutEdgeSort>
		void sortOutEdges();

		void initOffsets();
		void initIdToIndex();
		void setEdgeSrcTgtToOffset();

		template <class OutEdgeSort, class InEdgeSort>
		void update();

	public:
		Graph() : _next_id(0) {}

		/* Init the graph from file 'filename' and sort
		 * the edges according to OutEdgeSort and InEdgeSort. */
		template <class OutEdgeSort, class InEdgeSort>
		void init(GraphInData<Node,Edge>& data);

		class EdgeIt;
		class OffEdgeIt;

		/* Read the nodes and edges from the file <filename>.
		 * The offset vectors aren't initialized! */
		void printInfo() const;
		void printInfo(std::list<NodeID> const& nodes) const;

		uint getNrOfNodes() const;
		uint getNrOfEdges() const;
		uint getNrOfEdges(NodeID node_id) const;
		uint getNrOfEdges(NodeID node_id, EdgeType type) const;
		Edge const& getEdge(EdgeID edge_id) const;
		Node const& getNode(NodeID node_id) const;
		NodeID getOffId(NodeID node_id, EdgeType type) const;
		GraphCHOutData<Node,Edge> getData() const;

		friend void unit_tests::testGraph();
};

/*
 * Graph member functions.
 */

template <typename Node, typename Edge>
template <class OutEdgeSort, class InEdgeSort>
void Graph<Node, Edge>::init(GraphInData<Node,Edge>& data)
{
	_nodes.swap(data.nodes);
	_out_edges.swap(data.edges);
	_in_edges = _out_edges;
	_next_id = _out_edges.size();

	sortOutEdges<OutEdgeSort>();
	sortInEdges<InEdgeSort>();
	initOffsets();
	initIdToIndex();

	Print("Graph info:");
	Print("===========");
	printInfo();
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::printInfo() const
{
	std::list<NodeID> nodes;
	for (uint i(0), size(_nodes.size()); i<size; i++) {
		nodes.push_back(i);
	}

	printInfo(nodes);
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::printInfo(std::list<NodeID> const& nodes) const
{
	uint active_nodes(0);

	double avg_out_deg(0);
	double avg_in_deg(0);
	double avg_deg(0);

	std::vector<uint> out_deg;
	std::vector<uint> in_deg;
	std::vector<uint> deg;

	for (auto it(nodes.begin()), end(nodes.end()); it != end; it++) {
		uint out(getNrOfEdges(*it, OUT));
		uint in(getNrOfEdges(*it, IN));

		if (out != 0 || in != 0) {
			active_nodes++;

			out_deg.push_back(getNrOfEdges(*it, OUT));
			in_deg.push_back(getNrOfEdges(*it, IN));
			deg.push_back(out + in);

			avg_out_deg += out;
			avg_in_deg += in;
			avg_deg += out+in;
		}
	}

	Print("#nodes: " << nodes.size());
	Print("#active nodes: " << active_nodes);
	Print("#edges: " << _out_edges.size());
	Print("maximal edge id: " << _next_id-1);

	if (active_nodes != 0) {
		auto mm_out_deg = std::minmax_element(out_deg.begin(), out_deg.end());
		auto mm_in_deg = std::minmax_element(in_deg.begin(), in_deg.end());
		auto mm_deg = std::minmax_element(deg.begin(), deg.end());

		avg_out_deg /= active_nodes;
		avg_in_deg /= active_nodes;
		avg_deg /= active_nodes;

		Print("maximal out degree: " << *mm_out_deg.second);
		Print("minimal out degree: " << *mm_out_deg.first);
		Print("maximal in degree: " << *mm_in_deg.second);
		Print("minimal in degree: " << *mm_in_deg.first);
		Print("maximal degree: " << *mm_deg.second);
		Print("minimal degree: " << *mm_deg.first);
		Print("average out degree: " << avg_out_deg);
		Print("average in degree: " << avg_in_deg);
		Print("average degree: " << avg_deg);
		Print("(only degrees of active nodes are counted)");
	}
	else {
		Print("(no degree info is provided as there are no active nodes)");
	}
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
	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), EdgeSortSrc<Edge>()));
	assert(std::is_sorted(_in_edges.begin(), _in_edges.end(), EdgeSortTgt<Edge>()));

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

	_id_to_index.resize(_next_id);
	for (uint i(0), size(_out_edges.size()); i<size; i++) {
		_id_to_index[_out_edges[i].id] = i;
	}
}

template <typename Node, typename Edge>
template <class OutEdgeSort, class InEdgeSort>
void Graph<Node, Edge>::update()
{
	sortOutEdges<OutEdgeSort>();
	sortInEdges<InEdgeSort>();
	initOffsets();
	initIdToIndex();
}

template <typename Node, typename Edge>
void Graph<Node, Edge>::setEdgeSrcTgtToOffset()
{
	assert(_in_edges.size() == _out_edges.size());

	for (uint i(0), size(_in_edges.size()); i<size; i++) {
		_in_edges[i].src = _in_offsets[_in_edges[i].src];
		_in_edges[i].tgt = _out_offsets[_in_edges[i].tgt];
		_out_edges[i].src = _in_offsets[_out_edges[i].src];
		_out_edges[i].tgt = _out_offsets[_out_edges[i].tgt];
	}
}

template <typename Node, typename Edge>
Edge const& Graph<Node, Edge>::getEdge(EdgeID edge_id) const
{
	return _out_edges[_id_to_index[edge_id]];
}

template <typename Node, typename Edge>
Node const& Graph<Node, Edge>::getNode(NodeID node_id) const
{
	return _nodes[node_id];
}

template <typename Node, typename Edge>
NodeID Graph<Node, Edge>::getOffId(NodeID node_id, EdgeType type) const
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
uint Graph<Node, Edge>::getNrOfEdges(NodeID node_id) const
{
	return getNrOfEdges(node_id, OUT) + getNrOfEdges(node_id, IN);
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

template <typename Node, typename Edge>
GraphCHOutData<Node,Edge> Graph<Node,Edge>::getData() const
{
	return GraphCHOutData<Node,Edge>{_nodes, _out_edges};
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

}

#endif
