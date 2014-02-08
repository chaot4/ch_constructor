#ifndef _DIJKSTRA_H
#define _DIJKSTRA_H

#include "graph.h"

#include <vector>
#include <limits>
#include <queue>

namespace
{
	uint const NO_DIST(std::numeric_limits<uint>::max());
}

namespace unit_tests
{
	void testCHDijkstra();
	void testDijkstra();
}

template <typename Node, typename Edge>
class Dijkstra
{
	private:
		struct PQElement;
		typedef std::priority_queue<
				PQElement, std::vector<PQElement>, std::greater<PQElement> > PQ;

		Graph<Node, Edge> const& _g;

		std::vector<EdgeID> _found_by;
		std::vector<uint> _dists;
		std::vector<uint> _reset_dists;

		void _reset();
		void _relaxAllEdges(PQ& pq, PQElement const& top);
	public:
		Dijkstra(Graph<Node, Edge> const& g);

		/**
		 * @brief Computes the shortest path between src and tgt.
		 *
		 * @param path The edges of the shortest path in no
		 * particular order.
		 *
		 * @return The distance of the shortest path.
		 */
		uint calcShopa(NodeID src, NodeID tgt,
				std::vector<EdgeID>& path);
};

template <typename Node, typename Edge>
struct Dijkstra<Node, Edge>::PQElement
{
	NodeID node;
	EdgeID found_by;
	uint dist;

	PQElement(NodeID node, EdgeID found_by, uint dist)
		: node(node), found_by(found_by), dist(dist) {}

	bool operator>(PQElement const& other) const
	{
		return dist > other.dist;
	}
};

template <typename Node, typename Edge>
Dijkstra<Node,Edge>::Dijkstra(Graph<Node, Edge> const& g)
	: _g(g), _found_by(g.getNrOfNodes()),
	_dists(g.getNrOfNodes(), NO_DIST) {}

template <typename Node, typename Edge>
uint Dijkstra<Node,Edge>::calcShopa(NodeID src, NodeID tgt,
		std::vector<EdgeID>& path)
{
	_reset();
	path.clear();

	PQ pq;
	pq.push(PQElement(src, c::NO_EID, 0));
	_dists[src] = 0;
	_reset_dists.push_back(src);

	// Dijkstra loop
	while (!pq.empty() && pq.top().node != tgt) {

		PQElement top(pq.top());
		if (_dists[top.node] == top.dist) {
			_found_by[top.node] = top.found_by;
			_relaxAllEdges(pq, top);
		}

		pq.pop();
	}

	if (pq.empty()) {
		Print("No path found from " << src << " to " << tgt << ".");
		return NO_DIST;
	}

	// Path backtracking.
	NodeID bt_node(tgt);
	_found_by[tgt] = pq.top().found_by;
	while (bt_node != src) {
		Edge const& bt_edge(_g.getEdge(_found_by[bt_node]));
		bt_node = bt_edge.src;
		path.push_back(bt_edge.id);
	}

	return pq.top().dist;
}

template <typename Node, typename Edge>
void Dijkstra<Node,Edge>::_relaxAllEdges(PQ& pq, PQElement const& top)
{
	typename Graph<Node,Edge>::EdgeIt edge_it(_g, top.node, OUT);
	while (edge_it.hasNext()) {

		Edge const& edge(edge_it.getNext());
		NodeID tgt(edge.tgt);
		uint new_dist(top.dist + edge.dist);

		if (new_dist < _dists[tgt]) {

			if (_dists[tgt] == NO_DIST) {
				_reset_dists.push_back(tgt);
			}
			_dists[tgt] = new_dist;

			pq.push(PQElement(tgt, edge.id, new_dist));
		}
	}
}

template <typename Node, typename Edge>
void Dijkstra<Node,Edge>::_reset()
{
	for (uint i=0; i<_reset_dists.size(); i++) {
		_dists[_reset_dists[i]] = NO_DIST;
	}

	_reset_dists.clear();
}

class CHDijkstra
{
	private:
	public:
};

#endif
