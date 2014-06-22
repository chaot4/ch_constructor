#include "nodes_and_edges.h"

namespace chc
{
	NodeID Edge::otherNode(EdgeType edge_type) const
	{
		if (edge_type == OUT) {
			return tgt;
		}
		else {
			return src;
		}
	}

	CHEdge<Edge> Edge::concat(Edge const& edge1, Edge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return CHEdge<Edge>(Edge(c::NO_EID, edge1.src, edge2.tgt,
				edge1.dist + edge2.dist), edge1.id, edge2.id, edge1.tgt);
	}
}
