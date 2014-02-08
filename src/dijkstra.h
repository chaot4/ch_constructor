#ifndef _DIJKSTRA_H
#define _DIJKSTRA_H

#include "graph.h"
#include "chgraph.h"

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
	uint dist;

	PQElement(NodeID node, EdgeID found_by, EdgeType direction, uint dist)
		: node(node), found_by(found_by), direction(direction), dist(dist) {}

	bool operator>(PQElement const& other) const
	{
		return dist > other.dist;
	}
};

template <typename Node, typename Edge>
CHDijkstra<Node,Edge>::CHDijkstra(SCGraph<Node, Edge> const& g)
	: _g(g), _found_by(2, std::vector<EdgeID>(g.getNrOfNodes())),
	_dists(2, std::vector<uint>(g.getNrOfNodes(), NO_DIST)),
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
	// TODO zum struct zusammenfassen
	uint shortest_dist(NO_DIST);
	NodeID center_node(c::NO_NID);;
	while (!pq.empty() && pq.top().dist < shortest_dist) {

		PQElement top(pq.top());
		if (_dists[top.direction][top.node] == top.dist) {
			_found_by[top.direction][top.node] = top.found_by;
			_relaxAllEdges(pq, top);
		}

		pq.pop();
	}

	if (pq.empty()) {
		Print("No path found from " << src << " to " << tgt << ".");
		return NO_DIST;
	}

	// Path backtracking.
	for (uint i(0); i<2; i++) {
		NodeID bt_node(center_node);
		NodeID end_node;
		if (i == 0) {
			end_node = src;
		}
		else {
			end_node = tgt;
		}

		while (bt_node != end_node) {
			Edge const& bt_edge(_g.getEdge(_found_by[i][bt_node]));
			bt_node = bt_edge.other_node(i);
			path.push_back(bt_edge.id);
		}
	}

	return shortest_dist;
}

template <typename Node, typename Edge>
void CHDijkstra<Node,Edge>::_relaxAllEdges(PQ& pq, PQElement const& top)
{
	// TODO anpassen
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
void CHDijkstra<Node,Edge>::_reset()
{
	for(uint i=0; i<2; i++) {
		for (uint j=0; j<_reset_dists[i].size(); j++) {
			_dists[i][_reset_dists[i][j]] = NO_DIST;
		}
	}

	_reset_dists[0].clear();
	_reset_dists[1].clear();
}

#endif
