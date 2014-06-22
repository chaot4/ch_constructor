#include "nodes_and_edges.h"

namespace chc
{
	CHEdge<Edge> concat(Edge const& edge1, Edge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return CHEdge<Edge>(Edge(c::NO_EID, edge1.src, edge2.tgt,
				edge1.dist + edge2.dist), edge1.id, edge2.id, edge1.tgt);
	}

	CHEdge<OSMEdge> concat(OSMEdge const& edge1, OSMEdge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return CHEdge<OSMEdge>(OSMEdge(c::NO_EID, edge1.src, edge2.tgt,
				edge1.dist + edge2.dist, 0, -1), edge1.id, edge2.id, edge1.tgt);
	}
}
