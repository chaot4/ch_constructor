#pragma once

#include "graph.h"
#include "chgraph.h"
#include "enum_array.h"

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
		std::vector<NodeID> _reset_dists;

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
	for (auto const& edge: _g.nodeEdges(top.node, EdgeType::OUT)) {
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
	for (auto const node: _reset_dists) {
		_dists[node] = c::NO_DIST;
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

		CHGraph<Node, Edge> const& _g;

		/*
		 * data stored per direction
		 */
		struct direction_info {
			std::vector<EdgeID> _found_by;
			std::vector<uint> _dists;
			std::vector<NodeID> _reset_dists;
		};
		enum_array<direction_info, EdgeType, 2> _dir;

		void _reset();
		void _relaxAllEdges(PQ& pq, PQElement const& top);
	public:
		CHDijkstra(CHGraph<Node, Edge> const& g);

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
CHDijkstra<Node,Edge>::CHDijkstra(CHGraph<Node, Edge> const& g)
: _g(g) {
	for(auto& dir_info: _dir) {
		dir_info._dists.resize(g.getNrOfNodes(), c::NO_DIST);
		dir_info._found_by.resize(g.getNrOfNodes());
	}
}

template <typename Node, typename Edge>
uint CHDijkstra<Node,Edge>::calcShopa(NodeID src, NodeID tgt,
		std::vector<EdgeID>& path)
{
	_reset();
	path.clear();

	PQ pq;
	pq.push(PQElement(src, c::NO_EID, EdgeType::OUT, 0));
	pq.push(PQElement(tgt, c::NO_EID, EdgeType::IN, 0));
	_dir[EdgeType::OUT]._dists[src] = 0;
	_dir[EdgeType::OUT]._reset_dists.push_back(src);
	_dir[EdgeType::IN]._dists[tgt] = 0;
	_dir[EdgeType::IN]._reset_dists.push_back(tgt);

	// Dijkstra loop
	uint shortest_dist(c::NO_DIST);
	NodeID center_node(c::NO_NID);;
	while (!pq.empty() && pq.top().distance() <= shortest_dist) {
		PQElement top(pq.top());
		pq.pop();

		if (_dir[top.direction]._dists[top.node] == top.distance()) {
			_dir[top.direction]._found_by[top.node] = top.found_by;
			_relaxAllEdges(pq, top);

			uint rest_dist = _dir[!top.direction]._dists[top.node];
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
		if (dir == EdgeType::OUT) {
			end_node = src;
		}
		else {
			end_node = tgt;
		}

		while (bt_node != end_node) {
			EdgeID edge_id = _dir[dir]._found_by[bt_node];
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

			if (new_dist < _dir[dir]._dists[other_node]) {
				if (_dir[dir]._dists[other_node] == c::NO_DIST) {
					_dir[dir]._reset_dists.push_back(other_node);
				}
				_dir[dir]._dists[other_node] = new_dist;

				pq.push(PQElement(other_node, edge.id, dir, new_dist));
			}
		}
	}
}

template <typename Node, typename Edge>
void CHDijkstra<Node,Edge>::_reset()
{
	for (auto& dir: _dir) {
		for (auto const node: dir._reset_dists) {
			dir._dists[node] = c::NO_DIST;
		}
		dir._reset_dists.clear();
	}
}

}
