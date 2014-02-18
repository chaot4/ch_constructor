#ifndef _CHGRAPH_H
#define _CHGRAPH_H

#include "graph.h"
#include "nodes_and_edges.h"

#include <vector>
#include <algorithm>

namespace chc
{

template <typename Node, typename Edge>
class SCGraph : public Graph<CHNode<Node>, CHEdge<Edge> >
{
	private:
		typedef CHNode<Node> LvlNode;
		typedef CHEdge<Edge> Shortcut;
		typedef Graph<LvlNode, Shortcut> BaseGraph;
		using BaseGraph::_nodes;
		using BaseGraph::_out_edges;
		using BaseGraph::_in_edges;
		using BaseGraph::_id_to_index;
		using BaseGraph::_next_id;

		std::vector<Shortcut> _edges_dump;

		uint _next_lvl;

		void _addNewEdge(Shortcut& new_edge,
				std::vector<Shortcut>& new_edge_vec);
		void _addDumpEdge(Shortcut& new_edge);
	public:
		SCGraph() : BaseGraph(), _next_lvl(0) {}

		void restructure(std::vector<NodeID> const& deleted,
				std::vector<bool> const& to_delete,
				std::vector<Shortcut>& new_shortcuts);
		void buildCHGraph();

		bool isUp(EdgeID id, EdgeType direction) const;
};

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::restructure(
		std::vector<NodeID> const& deleted,
		std::vector<bool> const& to_delete,
		std::vector<Shortcut>& new_shortcuts)
{
	/*
	 * Process contracted nodes.
	 */
	for (uint i(0); i<deleted.size(); i++) {
		_nodes[deleted[i]].lvl = _next_lvl;
		assert(to_delete[deleted[i]]);
	}
	_next_lvl++;

	/*
	 * Process new shortcuts.
	 */
	std::vector<Shortcut> new_edge_vec;
	new_edge_vec.reserve(_out_edges.size() + new_shortcuts.size());

	std::sort(new_shortcuts.begin(), new_shortcuts.end(), EdgeSortSrc<Shortcut>());

	/* Manually merge the new_shortcuts and _out_edges vector. */
	uint j(0);
	for (uint i(0); i<_out_edges.size(); i++) {

		Shortcut const& edge(_out_edges[i]);
		/* edge greater */
		while (j < new_shortcuts.size() && new_shortcuts[j] < edge) {

			Shortcut& new_sc(new_shortcuts[j]);
			if (to_delete[new_sc.center_node]) {
				_addNewEdge(new_sc, new_edge_vec);
				assert(!to_delete[new_sc.src] && !to_delete[new_sc.tgt]);
			}
			j++;
		}

		/* edge equal */
		while (j < new_shortcuts.size() && new_shortcuts[j] == edge) {
			Shortcut& new_sc(new_shortcuts[j]);
			if (edge.dist >= new_sc.dist && to_delete[new_sc.center_node]) {
				_addNewEdge(new_sc, new_edge_vec);
				assert(!to_delete[new_sc.src] && !to_delete[new_sc.tgt]);
			}
			j++;
		}

		assert(j >= new_shortcuts.size() || edge < new_shortcuts[j]);

		/* edges smaller */
		if (!to_delete[edge.src] && !to_delete[edge.tgt]) {
			_addNewEdge(_out_edges[i], new_edge_vec);
		}
		else {
			_addDumpEdge(_out_edges[i]);
		}
	}

	/* Rest of new_shortcuts */
	while (j < new_shortcuts.size()) {

		Shortcut& new_sc(new_shortcuts[j]);
		if (to_delete[new_sc.center_node]) {
			_addNewEdge(new_sc, new_edge_vec);
			assert(!to_delete[new_sc.src] && !to_delete[new_sc.tgt]);
		}
		j++;
	}

	/*
	 * Build new graph structures.
	 */
	_out_edges.swap(new_edge_vec);
	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), EdgeSortSrc<Edge>()));

	_in_edges.assign(_out_edges.begin(), _out_edges.end());
	BaseGraph::template sortInEdges<EdgeSortTgt<Edge> >();
	BaseGraph::initOffsets();
}

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::_addNewEdge(Shortcut& new_edge,
		std::vector<Shortcut>& new_edge_vec)
{
	Edge& last_edge(new_edge_vec.back());
	if (!new_edge_vec.empty() && new_edge == last_edge) {
		if (new_edge.dist < last_edge.dist) {
			if (new_edge.id == c::NO_EID) {
				new_edge.id = _next_id++;
			}
			last_edge = new_edge;
		}
	}
	else {
		if (new_edge.id == c::NO_EID) {
			new_edge.id = _next_id++;
		}
		new_edge_vec.push_back(new_edge);
	}
}

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::_addDumpEdge(Shortcut& new_edge)
{
	Edge& last_edge(_edges_dump.back());
	if (!_edges_dump.empty() && new_edge == last_edge) {
		if (new_edge.dist < last_edge.dist) {
			last_edge = new_edge;
		}
	}
	else {
		_edges_dump.push_back(new_edge);
	}
}

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::buildCHGraph()
{
	assert(_out_edges.empty() && _in_edges.empty());

	_out_edges.swap(_edges_dump);
	_in_edges = _out_edges;
	_edges_dump.clear();

	BaseGraph::template update<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >();
}

template <typename Node, typename Edge>
bool SCGraph<Node, Edge>::isUp(EdgeID id, EdgeType direction) const
{
	Edge const& edge(_out_edges[_id_to_index[id]]);
	uint src_lvl = _nodes[edge.src].lvl;
	uint tgt_lvl = _nodes[edge.tgt].lvl;

	if (src_lvl > tgt_lvl) {
		return direction == IN ? true : false;
	}
	else if (src_lvl < tgt_lvl) {
		return direction == OUT ? true : false;
	}

	assert(src_lvl == tgt_lvl);
	return false;
}

}

#endif
