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
EdgeType operator!(EdgeType type);

/* Parser nodes predef */
struct STDNode;
struct SIMPLENode;

/* Parser edges predef */
struct STDEdge;
struct SIMPLEEdge;

/*
 * Nodes
 */

struct Node
{
	NodeID id;

	Node();
	Node(NodeID id);
	Node (STDNode const& node);
	Node (SIMPLENode const& node, NodeID id);

	bool operator<(Node const& node) const;
};

template<typename Node>
struct CHNode : Node
{
	uint lvl;

	CHNode()
		: Node(), lvl(c::NO_LVL){}
	CHNode(Node const& node, uint lvl)
		: Node(node), lvl(lvl){}
	CHNode(STDNode const& node)
		: Node(node), lvl(c::NO_LVL) {}
	CHNode(SIMPLENode const& node, NodeID id)
		: Node(node, id), lvl(c::NO_LVL) {}
};

/*
 * Edges
 */

template <typename Edge>
struct CHEdge;

struct Edge
{
	EdgeID id;
	NodeID src;
	NodeID tgt;
	uint dist;

	Edge();
	Edge(EdgeID id, NodeID src, NodeID tgt, uint dist);
	Edge(STDEdge const& edge, EdgeID id);
	Edge(SIMPLEEdge const& edge, EdgeID id);

	bool operator<(Edge const& edge) const;
	bool operator==(Edge const& edge) const;

	NodeID otherNode(EdgeType edge_type) const;
	static CHEdge<Edge> concat(Edge const& edge1, Edge const& edge2);
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
	EdgeID child_edge1;
	EdgeID child_edge2;
	NodeID center_node;

	CHEdge() : child_edge1(c::NO_EID), child_edge2(c::NO_EID),
			center_node(c::NO_NID){}
	CHEdge(Edge const& edge, EdgeID child_edge1, EdgeID child_edge2, NodeID center_node)
		: Edge(edge), child_edge1(child_edge1),
		child_edge2(child_edge2), center_node(center_node){}
	CHEdge(STDEdge const& edge, EdgeID id)
		: Edge(edge, id), child_edge1(c::NO_EID),
		child_edge2(c::NO_EID), center_node(c::NO_NID) {}
	CHEdge(SIMPLEEdge const& edge, EdgeID id)
		: Edge(edge, id), child_edge1(c::NO_EID),
		child_edge2(c::NO_EID), center_node(c::NO_NID) {}
};

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

/* 
 * Parser Nodes 
 */

struct STDNode
{
	NodeID id;
	uint osm_id;
	double lat;
	double lon;
	int elev;

	STDNode() {}
	STDNode(Node const& node);
};

struct SIMPLENode
{
	double lat;
	double lon;
	int elev;

	SIMPLENode() {}
	SIMPLENode(Node const& node);
};

/* 
 * Parser Edges
 */

struct STDEdge
{
	NodeID src;
	NodeID tgt;
	uint dist;
	uint type;
	int speed;

	STDEdge() {}
	STDEdge(Edge const& edge);
};

struct SIMPLEEdge
{
	NodeID src;
	NodeID tgt;
	uint dist;

	SIMPLEEdge() {}
	SIMPLEEdge(Edge const& node);
};

}

#endif
