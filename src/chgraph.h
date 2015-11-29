#pragma once

#include "graph.h"
#include "nodes_and_edges.h"

#include <vector>
#include <algorithm>

namespace chc
{

template <typename NodeT, typename EdgeT>
class CHGraph : public Graph<NodeT, CHEdge<EdgeT> >
{
	private:
		typedef CHEdge<EdgeT> Shortcut;
		typedef Graph<NodeT, Shortcut> BaseGraph;
		using BaseGraph::_out_edges;
		using BaseGraph::_in_edges;
		using BaseGraph::_id_to_index;
		using BaseGraph::edge_count;
		using typename BaseGraph::OutEdgeSort;

		std::vector<uint> _node_levels;

		std::vector<Shortcut> _edges_dump;

		uint _next_lvl = 0;

		void _addNewEdge(Shortcut& new_edge,
				std::vector<Shortcut>& new_edge_vec);
	public:
		template <typename Data>
		void init(Data&& data)
		{
			_node_levels.resize(data.nodes.size(), c::NO_LVL);
			BaseGraph::init(std::forward<Data>(data));
		}


		void restructure(std::vector<NodeID> const& removed,
				std::vector<bool> const& to_remove,
				std::vector<Shortcut>& new_shortcuts);
		void rebuildCompleteGraph();

		bool isUp(Shortcut const& edge, EdgeType direction) const;

		/* destroys internal data structures */
		GraphCHOutData<NodeT, Shortcut> exportData();
};

template <typename NodeT, typename EdgeT>
void CHGraph<NodeT, EdgeT>::restructure(
		std::vector<NodeID> const& removed,
		std::vector<bool> const& to_remove,
		std::vector<Shortcut>& new_shortcuts)
{
	BaseGraph::_is_dirty = true;

	OutEdgeSort outEdgeSort;

	/*
	 * Process contracted nodes.
	 */
	for (NodeID node: removed) {
		_node_levels[node] = _next_lvl;
		assert(to_remove[node]);
	}
	_next_lvl++;

	/*
	 * Process new shortcuts.
	 */
	std::vector<Shortcut> new_edge_vec;
	new_edge_vec.reserve(_out_edges.size() + new_shortcuts.size());

	std::sort(new_shortcuts.begin(), new_shortcuts.end(), outEdgeSort);

	/* Manually merge the new_shortcuts and _out_edges vector. */
	size_t j(0);
	for (auto& edge: _out_edges) {
		/* edge greater than new_sc */
		for (;j < new_shortcuts.size() && outEdgeSort(new_shortcuts[j], edge); ++j) {
			Shortcut& new_sc(new_shortcuts[j]);
			if (to_remove[new_sc.center_node]) {
				_addNewEdge(new_sc, new_edge_vec);
				assert(!to_remove[new_sc.src] && !to_remove[new_sc.tgt]);
			}
		}

		/* if edge and new_sc are "equal", i.e. have same endpoints, first add
		 * the old edge and overwrite on demand later
		 */

		debug_assert(j >= new_shortcuts.size() || !outEdgeSort(new_shortcuts[j], edge));

		/* edge less than or equal new_sc */
		if (!to_remove[edge.src] && !to_remove[edge.tgt]) {
			_addNewEdge(edge, new_edge_vec);
		}
		else {
			_edges_dump.push_back(edge);
		}
	}

	/* Rest of new_shortcuts */
	for (; j < new_shortcuts.size(); ++j) {
		Shortcut& new_sc(new_shortcuts[j]);
		if (to_remove[new_sc.center_node]) {
			_addNewEdge(new_sc, new_edge_vec);
			assert(!to_remove[new_sc.src] && !to_remove[new_sc.tgt]);
		}
	}

	/*
	 * Build new graph structures.
	 */
	_out_edges.swap(new_edge_vec);
	debug_assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), outEdgeSort));

	_in_edges.assign(_out_edges.begin(), _out_edges.end());
	BaseGraph::sortInEdges();
	BaseGraph::initOffsets();
}

template <typename NodeT, typename EdgeT>
void CHGraph<NodeT, EdgeT>::_addNewEdge(Shortcut& new_edge,
		std::vector<Shortcut>& new_edge_vec)
{
	BaseGraph::_is_dirty = true;

	if (!new_edge_vec.empty() && (c::NO_EID == new_edge.id)) {
		Shortcut& last_edge(new_edge_vec.back());

		if (equalEndpoints(new_edge, last_edge)) {
			/* ignore new edges if they are not better than what we already have */
			if (new_edge.distance() >= last_edge.distance()) return;

			/* only replace shortcut edges */
			if (c::NO_NID == last_edge.center_node) {
				/* reuse already assigned id */
				new_edge.id = last_edge.id;
				last_edge = new_edge;
				return;
			}
			/* otherwise simply add a new "duplicate" edge; don't remove
			 * edges from original graph - they might be useful for visualization
			 */
		}
	}

	if (c::NO_EID == new_edge.id) {
		new_edge.id = edge_count++;
	}
	new_edge_vec.push_back(new_edge);
}

template <typename NodeT, typename EdgeT>
void CHGraph<NodeT, EdgeT>::rebuildCompleteGraph()
{
	assert(_out_edges.empty() && _in_edges.empty());

	_out_edges.swap(_edges_dump);
	_in_edges = _out_edges;
	_edges_dump.clear();

	BaseGraph::update();
}

template <typename NodeT, typename EdgeT>
bool CHGraph<NodeT, EdgeT>::isUp(Shortcut const& edge, EdgeType direction) const
{
	uint src_lvl = _node_levels[edge.src];
	uint tgt_lvl = _node_levels[edge.tgt];

	if (src_lvl > tgt_lvl) {
		return direction == EdgeType::IN ? true : false;
	}
	else if (src_lvl < tgt_lvl) {
		return direction == EdgeType::OUT ? true : false;
	}

	/* should never reach this: */
	assert(src_lvl != tgt_lvl);
	return false;
}

template <typename NodeT, typename EdgeT>
auto CHGraph<NodeT, EdgeT>::exportData() -> GraphCHOutData<NodeT, Shortcut>
{
	BaseGraph::_is_dirty = true;

	std::vector<Shortcut>* edges_source;
	std::vector<Shortcut> edges;

	_id_to_index = decltype(_id_to_index)();

	if (_out_edges.empty() && _in_edges.empty()) {
		edges_source = &_edges_dump;
	}
	else {
		assert(_edges_dump.empty());
		edges_source = &_out_edges;
		_in_edges = decltype(_in_edges)();
	}

	edges.resize(edges_source->size());
	for (auto const& edge: *edges_source) {
		edges[edge.id] = edge;
	}

	/* Sort edges for output and adapt id's */
	std::sort(edges.begin(), edges.end(), EdgeSortSrcTgt<EdgeT>());
	std::vector<size_t> new_id(edges.size());
	for (uint i(0); i<edges.size(); i++) {
		new_id[edges[i].id] = i;
	}
	for (uint i(0); i<edges.size(); i++) {
		Shortcut& edge(edges[i]);
		edge.id = new_id[i];
		edge.child_edge1 = (edge.child_edge1 != c::NO_EID ? new_id[edge.child_edge1] : c::NO_EID);
		edge.child_edge2 = (edge.child_edge2 != c::NO_EID ? new_id[edge.child_edge2] : c::NO_EID);
	}

	_out_edges = std::move(edges);

	_edges_dump = decltype(_edges_dump)();

	return GraphCHOutData<NodeT, Shortcut>{BaseGraph::_nodes, _node_levels, _out_edges, BaseGraph::_meta_data};
}

}
