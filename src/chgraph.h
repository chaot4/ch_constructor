#ifndef _CHGRAPH_H
#define _CHGRAPH_H

#include "graph.h"
#include "nodes_and_edges.h"

#include <vector>
#include <algorithm>

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

	public:
		SCGraph() : BaseGraph(), _next_lvl(0) {}

		void restructure(std::vector<NodeID> const& contracted_nodes,
				std::vector<bool> const& deleted,
				std::vector<Shortcut>& new_shortcuts);
		void buildCHGraph();

		bool isUp(EdgeID id, EdgeType direction) const;
};

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::restructure(
		std::vector<NodeID> const& contracted_nodes,
		std::vector<bool> const& deleted,
		std::vector<Shortcut>& new_shortcuts)
{
	/*
	 * Process contracted nodes.
	 */
	for (uint i(0); i<contracted_nodes.size(); i++) {
		_nodes[contracted_nodes[i]].lvl = _next_lvl;
	}
	_next_lvl++;

	/*
	 * Process new shortcuts.
	 */
	std::vector<Shortcut> new_edge_vec;
	new_edge_vec.reserve(_out_edges.size() + new_shortcuts.size());

	std::sort(new_shortcuts.begin(), new_shortcuts.end(), EdgeSortSrc<Shortcut>());
	new_shortcuts.erase(std::unique(new_shortcuts.begin(),
			new_shortcuts.end()), new_shortcuts.end());

	/* Manually merge the new_shortcuts and _out_edges vector. */
	uint j(0);
	for (uint i(0); i<_out_edges.size(); i++) {

		Shortcut const& edge(_out_edges[i]);
		/* edge greater */
		while (j < new_shortcuts.size() && new_shortcuts[j] < edge) {

			Shortcut& new_sc(new_shortcuts[j]);
			new_sc.id = _next_id++;
			new_edge_vec.push_back(new_sc);
			j++;

			assert(!deleted[new_sc.src] && !deleted[new_sc.tgt]);
		}

		/* edge equal */
		while (j < new_shortcuts.size() && new_shortcuts[j] == edge) {
			j++;
		}

		assert(j >= new_shortcuts.size() || edge < new_shortcuts[j]);

		/* edges smaller */
		if (!deleted[edge.src] && !deleted[edge.tgt]) {
			new_edge_vec.push_back(_out_edges[i]);
		}
		else {
			_edges_dump.push_back(_out_edges[i]);
		}
	}

	/* Rest of new_shortcuts */
	while (j < new_shortcuts.size()) {

		Shortcut& new_sc(new_shortcuts[j]);
		new_sc.id = _next_id++;
		new_edge_vec.push_back(new_sc);
		j++;

		assert(!deleted[new_sc.src] && !deleted[new_sc.tgt]);
	}

	_out_edges.swap(new_edge_vec);
	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), EdgeSortSrc<Edge>()));

	_in_edges.assign(_out_edges.begin(), _out_edges.end());
	BaseGraph::template sortInEdges<EdgeSortTgt<Edge> >();
	BaseGraph::initOffsets();
	BaseGraph::initIdToIndex();
}

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::buildCHGraph()
{
	_out_edges.swap(_edges_dump);
	_in_edges.assign(_out_edges.begin(), _out_edges.end());
	_edges_dump.clear();

	BaseGraph::template sortOutEdges<EdgeSortSrc<Edge> >();
	BaseGraph::template sortInEdges<EdgeSortTgt<Edge> >();
	BaseGraph::initOffsets();
	BaseGraph::initIdToIndex();
}

template <typename Node, typename Edge>
bool SCGraph<Node, Edge>::isUp(EdgeID id, EdgeType direction) const
{
	Edge const& edge(_out_edges[_id_to_index[id]]);
	uint src_lvl = _nodes[edge.src].lvl;
	uint tgt_lvl = _nodes[edge.tgt].lvl;

	if (src_lvl > tgt_lvl) {
		return direction;
	}
	else if (src_lvl < tgt_lvl) {
		return !direction;
	}
	return false;
}

#endif
