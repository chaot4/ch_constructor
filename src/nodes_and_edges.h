#ifndef _NODES_AND_EDGES_H
#define _NODES_AND_EDGES_H

#include "defs.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <limits>

namespace chc
{

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
	uint const NO_LVL(std::numeric_limits<uint>::max());
}

enum EdgeType {OUT = 0, IN = 1};
inline EdgeType operator!(EdgeType type) { return (EdgeType)(1 - type); }

/* Data required to construct or write out graphs. */
template <typename NodeT, typename EdgeT>
struct GraphInData {
	/* graph will "steal" data */
	std::vector<NodeT> nodes;
	std::vector<EdgeT> edges;
};
template <typename NodeT, typename EdgeT>
struct GraphCHOutData {
	std::vector<NodeT> const& nodes;
	std::vector<EdgeT> const& edges;
};


/*
 * Nodes
 */

struct Node
{
	NodeID id = c::NO_NID;

	explicit Node() { }
	explicit Node(NodeID id) : id(id) { }

	bool operator<(Node const& node) const { return id < node.id; }
};

template<typename NodeT>
struct CHNode : NodeT
{
	uint lvl = c::NO_LVL;

	explicit CHNode() { }
	explicit CHNode(NodeT const& node) : NodeT(node) { } 
	explicit CHNode(NodeT const& node, uint lvl) : NodeT(node), lvl(lvl){}
};

/*
 * Edges
 */

template <typename Edge>
struct CHEdge;

struct Edge
{
	EdgeID id = c::NO_NID;
	NodeID src = c::NO_NID;
	NodeID tgt = c::NO_NID;
	uint dist = c::NO_DIST;

	explicit  Edge() { }
	explicit  Edge(EdgeID id, NodeID src, NodeID tgt, uint dist)
		: id(id), src(src), tgt(tgt), dist(dist) { }

	bool operator<(Edge const& edge) const
	{
		return src < edge.src || (src == edge.src && tgt < edge.tgt);
	}

	bool operator==(Edge const& edge) const {
		return src == edge.src && tgt == edge.tgt;
	}

	NodeID otherNode(EdgeType edge_type) const;
	static CHEdge<Edge> concat(Edge const& edge1, Edge const& edge2);
};

template <typename EdgeT>
struct MetricEdge : EdgeT
{
	uint metric = 0;

	explicit MetricEdge() {}
	explicit MetricEdge(EdgeT const& edge) : EdgeT(edge) {}
	explicit MetricEdge(EdgeT const& edge, uint metric) : EdgeT(edge), metric(metric){}
};

template <typename EdgeT>
struct CHEdge : EdgeT
{
	EdgeID child_edge1 = c::NO_EID;
	EdgeID child_edge2 = c::NO_EID;
	NodeID center_node = c::NO_NID;

	explicit CHEdge() { }
	explicit CHEdge(EdgeT const& edge): EdgeT(edge) { }
	explicit CHEdge(EdgeT const& edge, EdgeID child_edge1, EdgeID child_edge2, NodeID center_node)
		: EdgeT(edge), child_edge1(child_edge1),
		child_edge2(child_edge2), center_node(center_node){}
};

struct OSMNode
{
	NodeID id = c::NO_NID;
	uint osm_id = 0;
	double lat = 0;
	double lon = 0;
	int elev = 0;

	explicit OSMNode() { }

	/* is this really a good idea? */
	explicit OSMNode(Node node) : id(node.id) { }

	operator Node() const
	{
		return Node(id);
	}

	bool operator<(OSMNode const& other) const { return id < other.id; }
};

struct OSMEdge
{
	NodeID id = c::NO_NID;
	NodeID src = c::NO_NID;
	NodeID tgt = c::NO_NID;
	uint dist = c::NO_DIST;
	uint type = 0;
	int speed = -1;

	explicit OSMEdge() { }

	/* is this really a good idea? */
	explicit OSMEdge(Edge edge) : id(edge.id), src(edge.src), tgt(edge.tgt), dist(edge.dist) { }

	operator Edge() const
	{
		return Edge(id, src, tgt, dist);
	}

	bool operator<(OSMEdge const& other) const
	{
		return src < other.src || (src == other.src && tgt < other.tgt);
	}
};

struct GeoNode
{
	NodeID id = c::NO_NID;
	double lat = 0;
	double lon = 0;
	int elev = 0;

	explicit GeoNode() { }

	/* is this really a good idea? */
	explicit GeoNode(Node node) : id(node.id) { }

	operator Node() const
	{
		return Node(id);
	}

	bool operator<(GeoNode const& other) const { return id < other.id; }
};


/*
 * EdgeSort
 */

template <typename EdgeT>
struct EdgeSortSrc
{
	bool operator()(EdgeT const& edge1, EdgeT const& edge2) const
	{
		return edge1.src < edge2.src ||
		       	(edge1.src == edge2.src && edge1.tgt < edge2.tgt);
	}
};

template <typename EdgeT>
struct EdgeSortTgt
{
	bool operator()(EdgeT const& edge1, EdgeT const& edge2) const
	{
		return edge1.tgt < edge2.tgt ||
		       	(edge1.tgt == edge2.tgt && edge1.src < edge2.src);
	}
};

}

#endif
