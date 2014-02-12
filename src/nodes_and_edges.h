#ifndef _NODES_AND_EDGES_H
#define _NODES_AND_EDGES_H

#include "defs.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <limits>

namespace unit_tests
{
	void testNodesAndEdges();
}

typedef uint NodeID;
typedef uint EdgeID;
namespace c
{
	uint const NO_NID(std::numeric_limits<NodeID>::max());
	uint const NO_EID(std::numeric_limits<EdgeID>::max());
	uint const NO_DIST(std::numeric_limits<uint>::max());
}

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

	virtual void read(std::stringstream& ss);
	virtual void write(std::ofstream& f) const;
	bool operator<(Node const& node) const { return id < node.id; }
};

void Node::read(std::stringstream& ss)
{
	Parser_Node node;
	ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;

	id = node.id;
}

void Node::write(std::ofstream& f) const
{
	f << id << " 0 0 0 0";
}

template<typename Node>
struct CHNode : Node
{
	uint lvl;

	CHNode() : Node(), lvl(0){}
	CHNode(Node const& node, uint lvl) : Node(node), lvl(lvl){}

	virtual void write(std::ofstream& f) const;
};

template<typename Node>
void CHNode<Node>::write(std::ofstream& f) const
{
	Node::write(f);
	f << " " << lvl;
}

/*
 * Edges
 */

template <typename Edge>
struct CHEdge;

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

	Edge() : id(c::NO_NID), src(c::NO_NID), tgt(c::NO_NID), dist(c::NO_DIST){}
	Edge(EdgeID id, NodeID src, NodeID tgt, uint dist)
		: id(id), src(src), tgt(tgt), dist(dist){}

	bool operator<(Edge const& edge) const
	{ 
		return src < edge.src || (src == edge.src && tgt < edge.tgt);
	}

	bool operator==(Edge const& edge) const
	{ 
		return src == edge.src && tgt == edge.tgt;
	}

	virtual void read(std::stringstream& ss);
	virtual void write(std::ofstream& f) const;
	NodeID otherNode(EdgeType edge_type) const;

	CHEdge<Edge> concat(Edge const& edge) const;
};

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
	int child_edge1;
	int child_edge2;
	NodeID center_node;

	CHEdge() : child_edge1(-1), child_edge2(-1), center_node(c::NO_NID){}
	CHEdge(Edge const& edge, EdgeID child_edge1, EdgeID child_edge2,
			NodeID center_node)
		: Edge(edge), child_edge1(child_edge1),
		child_edge2(child_edge2), center_node(center_node){}

	virtual void write(std::ofstream& f) const;
};

void Edge::read(std::stringstream& ss)
{
	Parser_Edge edge;
	ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;

	src = edge.src;
	tgt = edge.tgt;
	dist = edge.dist;
}

void Edge::write(std::ofstream& f) const
{
	f << src << " " << tgt << " " << dist << " 0 -1";
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

CHEdge<Edge> Edge::concat(Edge const& edge) const
{
	assert(tgt == edge.src);
	return CHEdge<Edge>(Edge(c::NO_EID, src, edge.tgt, dist + edge.dist), id, edge.id, tgt);
}

template <typename Edge>
void CHEdge<Edge>::write(std::ofstream& f) const
{
	Edge::write(f);
	f << " " << child_edge1 << " " << child_edge2;
}

/*
 * EdgeSort
 */

template <typename Edge>
struct EdgeSortSrc
{
	bool operator()(Edge const& edge1, Edge const& edge2) const
	{
		return edge1.src < edge2.src ||
		       	(edge1.src == edge2.src && edge1.tgt < edge2.tgt);
	}
};

template <typename Edge>
struct EdgeSortTgt
{
	bool operator()(Edge const& edge1, Edge const& edge2) const
	{
		return edge1.tgt < edge2.tgt ||
		       	(edge1.tgt == edge2.tgt && edge1.src < edge2.src);
	}
};

#endif
