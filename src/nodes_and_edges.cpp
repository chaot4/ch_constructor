#include "nodes_and_edges.h"

namespace chc
{
	Edge concat(Edge const& edge1, Edge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return Edge(c::NO_EID, edge1.src, edge2.tgt, edge1.dist + edge2.dist);
	}

	OSMEdge concat(OSMEdge const& edge1, OSMEdge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return OSMEdge(c::NO_EID, edge1.src, edge2.tgt, edge1.dist + edge2.dist, 0, -1);
	}

	EuclOSMEdge concat(EuclOSMEdge const& edge1, EuclOSMEdge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return EuclOSMEdge(c::NO_EID, edge1.src, edge2.tgt, edge1.dist + edge2.dist, 0, -1,
				edge1.eucl_dist + edge2.eucl_dist);
	}

	StefanEdge concat(StefanEdge const& edge1, StefanEdge const& edge2)
	{
		assert(edge1.tgt == edge2.src);
		return StefanEdge(c::NO_EID, edge1.src, edge2.tgt, edge1.dist + edge2.dist);
	}
}
