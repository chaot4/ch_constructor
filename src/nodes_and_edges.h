#ifndef _NODES_AND_EDGES_H
#define _NODES_AND_EDGES_H

#include "defs.h"

#include <sstream>
#include <vector>

namespace unit_tests
{
	void testNodesAndEdges();
}

typedef uint NodeID;
typedef uint EdgeID;

enum EdgeType {OUT = 0, IN = 1};

EdgeType operator!(EdgeType type)
{
	if (type == IN) {
		return OUT;
	}
	else {
		return IN;
	}
}

/*
 * Nodes
 */

struct Parser_Node{
	NodeID id;
	uint osm_id;
	double lat;
	double lon;
	int elev;
};

struct Node
{
	NodeID id;

	Node() : id(0){}
	Node(NodeID id) : id(id){}

	void read(std::stringstream& ss);
	bool operator<(Node const& node) const { return id < node.id; }
};

void Node::read(std::stringstream& ss)
{
	Parser_Node node;
	ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;

	id = node.id;
}

struct CHNode : Node
{
	uint lvl;

	CHNode() : Node(), lvl(0){}
	CHNode(Node const& node, uint lvl) : Node(node), lvl(lvl){}
};

/*
 * Edges
 */

struct Parser_Edge{
	NodeID src;
	NodeID tgt;
	uint dist;
	uint type;
	int speed;
};

struct Edge
{
	EdgeID id;
	NodeID src;
	NodeID tgt;
	uint dist;

	Edge() : id(0), src(0), tgt(0), dist(0){}
	Edge(EdgeID id, NodeID src, NodeID tgt, uint dist)
		: id(id), src(src), tgt(tgt), dist(dist){}

	void read(std::stringstream& ss);
	NodeID otherNode(EdgeType edge_type) const;
};

void Edge::read(std::stringstream& ss)
{
	Parser_Edge edge;
	ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;

	src = edge.src;
	tgt = edge.tgt;
	dist = edge.dist;
}

NodeID Edge::otherNode(EdgeType edge_type) const
{
	if (edge_type == OUT) {
		return tgt;
	}
	else {
		return src;
	}
}

template <typename Edge>
struct MetricEdge : Edge
{
	uint metric;

	MetricEdge() : Edge(), metric(0){}
	MetricEdge(Edge const& edge, uint metric) : Edge(edge), metric(metric){}
};

template <typename Edge>
struct CHEdge : Edge
{
	EdgeID child_edge1;
	EdgeID child_edge2;

	CHEdge() : child_edge1(0), child_edge2(0){}
	CHEdge(Edge const& edge, EdgeID child_edge1, EdgeID child_edge2)
		: Edge(edge), child_edge1(child_edge1), child_edge2(child_edge2){}
};

/*
 * EdgeSort
 */

template <typename Edge>
struct EdgeSortSrc
{
	bool operator()(Edge const& edge1, Edge const& edge2) const
	{
		return edge1.src < edge2.src;
	}
};

template <typename Edge>
struct EdgeSortTgt
{
	bool operator()(Edge const& edge1, Edge const& edge2) const
	{
		return edge1.tgt < edge2.tgt;
	}
};

#endif
