#include "nodes_and_edges.h"

namespace chc
{

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

Node::Node()
	: id(c::NO_NID) {}

Node::Node(NodeID id)
	: id(id) {}

Node::Node (Format1Node const& node)
	: id(node.id) {}

bool Node::operator<(Node const& node) const
{
	return id < node.id;
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

/*
 * Edges
 */

Edge::Edge()
	: id(c::NO_NID), src(c::NO_NID), tgt(c::NO_NID),
	dist(c::NO_DIST){}

Edge::Edge(EdgeID id, NodeID src, NodeID tgt, uint dist)
	: id(id), src(src), tgt(tgt), dist(dist){}

Edge::Edge(Format1Edge const& edge, EdgeID id)
	: id(id), src(edge.src), tgt(edge.tgt), dist(edge.dist) {}

bool Edge::operator<(Edge const& edge) const
{ 
	return src < edge.src || (src == edge.src && tgt < edge.tgt);
}

bool Edge::operator==(Edge const& edge) const
{ 
	return src == edge.src && tgt == edge.tgt;
}

CHEdge<Edge> Edge::concat(Edge const& edge1, Edge const& edge2)
{
	assert(edge1.tgt == edge2.src);
	return CHEdge<Edge>(Edge(c::NO_EID, edge1.src, edge2.tgt,
			edge1.dist + edge2.dist), edge1.id, edge2.id, edge1.tgt);
}

/*
 * Parser Nodes
 */

Format1Node::Format1Node(Node const& node)
	: id(node.id), osm_id(0), lat(0), lon(0), elev(0) {}

/*
 * Parser Edges
 */

Format1Edge::Format1Edge(Edge const& edge)
	: src(edge.src), tgt(edge.tgt), dist(edge.dist), type(0), speed(-1) {}

}
