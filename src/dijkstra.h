#pragma once

#include "graph.h"
#include "chgraph.h"

#include <vector>
#include <limits>
#include <queue>

namespace chc
{

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
	uint _dist;

	PQElement(NodeID node, EdgeID found_by, uint dist)
		: node(node), found_by(found_by), _dist(dist) {}

	bool operator>(PQElement const& other) const
	{
		return _dist > other._dist;
	}

	/* make interface look similar to an edge */
	uint distance() const { return _dist; }
};

template <typename Node, typename Edge>
Dijkstra<Node,Edge>::Dijkstra(Graph<Node, Edge> const& g)
	: _g(g), _found_by(g.getNrOfNodes()),
	_dists(g.getNrOfNodes(), c::NO_DIST) {}

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
		pq.pop();

		if (_dists[top.node] == top.distance()) {
			_found_by[top.node] = top.found_by;
			_relaxAllEdges(pq, top);
		}
	}

	if (pq.empty()) {
		Print("No path found from " << src << " to " << tgt << ".");
		return c::NO_DIST;
	}

	// Path backtracking.
	NodeID bt_node(tgt);
	_found_by[tgt] = pq.top().found_by;
	while (bt_node != src) {
		EdgeID edge_id = _found_by[bt_node];
		bt_node = _g.getEdge(edge_id).src;
		path.push_back(edge_id);
	}

	return pq.top().distance();
}

template <typename Node, typename Edge>
void Dijkstra<Node,Edge>::_relaxAllEdges(PQ& pq, PQElement const& top)
{
	for (auto const& edge: _g.nodeEdges(top.node, OUT)) {
		NodeID tgt(edge.tgt);
		uint new_dist(top.distance() + edge.distance());

		if (new_dist < _dists[tgt]) {
			if (_dists[tgt] == c::NO_DIST) {
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
	for (uint i(0), size(_reset_dists.size()); i<size; i++) {
		_dists[_reset_dists[i]] = c::NO_DIST;
	}

	_reset_dists.clear();
}

template <typename Node, typename Edge>
class CHDijkstra
{
	private:
		struct PQElement;
		typedef std::priority_queue<
			PQElement, std::vector<PQElement>, std::greater<PQElement> > PQ;

		SCGraph<Node, Edge> const& _g;

		std::vector<std::vector<EdgeID> > _found_by;
		std::vector<std::vector<uint> > _dists;
		std::vector<std::vector<uint> > _reset_dists;

		void _reset();
		void _relaxAllEdges(PQ& pq, PQElement const& top);
	public:
		CHDijkstra(SCGraph<Node, Edge> const& g);

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
struct CHDijkstra<Node, Edge>::PQElement
{
	NodeID node;
	EdgeID found_by;
	EdgeType direction;
	uint _dist;

	PQElement(NodeID node, EdgeID found_by, EdgeType direction, uint dist)
		: node(node), found_by(found_by), direction(direction), _dist(dist) {}

	bool operator>(PQElement const& other) const
	{
		return _dist > other._dist;
	}

	/* make interface look similar to an edge */
	uint distance() const { return _dist; }
};

template <typename Node, typename Edge>
CHDijkstra<Node,Edge>::CHDijkstra(SCGraph<Node, Edge> const& g)
	: _g(g), _found_by(2, std::vector<EdgeID>(g.getNrOfNodes())),
	_dists(2, std::vector<uint>(g.getNrOfNodes(), c::NO_DIST)),
	_reset_dists(2) {}

template <typename Node, typename Edge>
uint CHDijkstra<Node,Edge>::calcShopa(NodeID src, NodeID tgt,
		std::vector<EdgeID>& path)
{
	_reset();
	path.clear();

	PQ pq;
	pq.push(PQElement(src, c::NO_EID, OUT, 0));
	pq.push(PQElement(tgt, c::NO_EID, IN, 0));
	_dists[OUT][src] = 0;
	_reset_dists[OUT].push_back(src);
	_dists[IN][tgt] = 0;
	_reset_dists[IN].push_back(tgt);

	// Dijkstra loop
	uint shortest_dist(c::NO_DIST);
	NodeID center_node(c::NO_NID);;
	while (!pq.empty() && pq.top().distance() <= shortest_dist) {
		PQElement top(pq.top());
		pq.pop();

		if (_dists[top.direction][top.node] == top.distance()) {
			_found_by[top.direction][top.node] = top.found_by;
			_relaxAllEdges(pq, top);

			uint rest_dist = _dists[!top.direction][top.node];
			if (rest_dist != c::NO_DIST
					&& top.distance() + rest_dist < shortest_dist) {
				shortest_dist = top.distance() + rest_dist;
				center_node = top.node;
			}
		}
	}

	if (center_node == c::NO_NID) {
		Print("No path found from " << src << " to " << tgt << ".");
		return c::NO_DIST;
	}

	// Path backtracking.
	for (uint i(0); i<2; i++) {
		EdgeType dir((EdgeType) i);
		NodeID bt_node(center_node);
		NodeID end_node;
		if (dir == OUT) {
			end_node = src;
		}
		else {
			end_node = tgt;
		}

		while (bt_node != end_node) {
			EdgeID edge_id = _found_by[dir][bt_node];
			bt_node = otherNode(_g.getEdge(edge_id), !dir);
			path.push_back(edge_id);
		}
	}

	return shortest_dist;
}

template <typename Node, typename Edge>
void CHDijkstra<Node,Edge>::_relaxAllEdges(PQ& pq, PQElement const& top)
{
	EdgeType dir(top.direction);
	// TODO When edges are sorted accordingly: loop while
	// edge is up.
	for (auto const& edge: _g.nodeEdges(top.node, dir)) {
		if (_g.isUp(edge, dir)) {
			NodeID other_node(otherNode(edge, dir));
			uint new_dist(top.distance() + edge.distance());

			if (new_dist < _dists[dir][other_node]) {
				if (_dists[dir][other_node] == c::NO_DIST) {
					_reset_dists[dir].push_back(other_node);
				}
				_dists[dir][other_node] = new_dist;

				pq.push(PQElement(other_node, edge.id, dir, new_dist));
			}
		}
	}
}

template <typename Node, typename Edge>
void CHDijkstra<Node,Edge>::_reset()
{
	for(uint i=0; i<2; i++) {
		for (uint j(0), size(_reset_dists[i].size()); j<size; j++) {
			_dists[i][_reset_dists[i][j]] = c::NO_DIST;
		}
	}

	_reset_dists[0].clear();
	_reset_dists[1].clear();
}

}
