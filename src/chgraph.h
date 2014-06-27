#ifndef _CHGRAPH_H
#define _CHGRAPH_H

#include "graph.h"
#include "nodes_and_edges.h"

#include <vector>
#include <algorithm>

namespace chc
{

template <typename NodeT, typename EdgeT>
class SCGraph : public Graph<NodeT, CHEdge<EdgeT> >
{
	private:
		typedef CHEdge<EdgeT> Shortcut;
		typedef Graph<NodeT, Shortcut> BaseGraph;
		using BaseGraph::_out_edges;
		using BaseGraph::_in_edges;
		using BaseGraph::_id_to_index;
		using BaseGraph::_next_id;
		using typename BaseGraph::OutEdgeSort;

		std::vector<uint> _node_levels;

		std::vector<Shortcut> _edges_dump;

		uint _next_lvl = 0;

		void _addNewEdge(Shortcut& new_edge,
				std::vector<Shortcut>& new_edge_vec);
		void _addDumpEdge(Shortcut& new_edge);
	public:
		template <typename Data>
		void init(Data&& data)
		{
			_node_levels.resize(data.nodes.size(), c::NO_LVL);
			BaseGraph::init(std::forward<Data>(data));
		}


		void restructure(std::vector<NodeID> const& deleted,
				std::vector<bool> const& to_delete,
				std::vector<Shortcut>& new_shortcuts);
		void rebuildCompleteGraph();

		bool isUp(Shortcut const& edge, EdgeType direction) const;

		GraphCHOutData<NodeT, Shortcut> getData() const
		{
			return GraphCHOutData<NodeT, Shortcut>{BaseGraph::_nodes, _node_levels, _out_edges};
		}
};

template <typename NodeT, typename EdgeT>
void SCGraph<NodeT, EdgeT>::restructure(
		std::vector<NodeID> const& deleted,
		std::vector<bool> const& to_delete,
		std::vector<Shortcut>& new_shortcuts)
{
	OutEdgeSort outEdgeSort;

	/*
	 * Process contracted nodes.
	 */
	for (uint i(0); i<deleted.size(); i++) {
		_node_levels[deleted[i]] = _next_lvl;
		assert(to_delete[deleted[i]]);
	}
	_next_lvl++;

	/*
	 * Process new shortcuts.
	 */
	std::vector<Shortcut> new_edge_vec;
	new_edge_vec.reserve(_out_edges.size() + new_shortcuts.size());

	std::sort(new_shortcuts.begin(), new_shortcuts.end(), outEdgeSort);

	/* Manually merge the new_shortcuts and _out_edges vector. */
	uint j(0);
	for (uint i(0); i<_out_edges.size(); i++) {

		Shortcut const& edge(_out_edges[i]);
		/* edge greater */
		while (j < new_shortcuts.size() && outEdgeSort(new_shortcuts[j], edge)) {

			Shortcut& new_sc(new_shortcuts[j]);
			if (to_delete[new_sc.center_node]) {
				_addNewEdge(new_sc, new_edge_vec);
				assert(!to_delete[new_sc.src] && !to_delete[new_sc.tgt]);
			}
			j++;
		}

		/* edge equal */
		while (j < new_shortcuts.size() && equalEndpoints(new_shortcuts[j], edge)) {
			Shortcut& new_sc(new_shortcuts[j]);
			if (edge.distance() >= new_sc.distance() && to_delete[new_sc.center_node]) {
				_addNewEdge(new_sc, new_edge_vec);
				assert(!to_delete[new_sc.src] && !to_delete[new_sc.tgt]);
			}
			j++;
		}

		assert(j >= new_shortcuts.size() || outEdgeSort(edge, new_shortcuts[j]));

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
	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), BaseGraph::OutEdgeSort()));

	_in_edges.assign(_out_edges.begin(), _out_edges.end());
	BaseGraph::sortInEdges();
	BaseGraph::initOffsets();
}

template <typename NodeT, typename EdgeT>
void SCGraph<NodeT, EdgeT>::_addNewEdge(Shortcut& new_edge,
		std::vector<Shortcut>& new_edge_vec)
{
	EdgeT& last_edge(new_edge_vec.back());
	if (!new_edge_vec.empty() && equalEndpoints(new_edge, last_edge)) {
		if (new_edge.distance() < last_edge.distance()) {
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

template <typename NodeT, typename EdgeT>
void SCGraph<NodeT, EdgeT>::_addDumpEdge(Shortcut& new_edge)
{
	EdgeT& last_edge(_edges_dump.back());
	if (!_edges_dump.empty() && equalEndpoints(new_edge, last_edge)) {
		if (new_edge.distance() < last_edge.distance()) {
			last_edge = new_edge;
		}
	}
	else {
		_edges_dump.push_back(new_edge);
	}
}

template <typename NodeT, typename EdgeT>
void SCGraph<NodeT, EdgeT>::rebuildCompleteGraph()
{
	assert(_out_edges.empty() && _in_edges.empty());

	_out_edges.swap(_edges_dump);
	_in_edges = _out_edges;
	_edges_dump.clear();

	BaseGraph::update();
}

template <typename NodeT, typename EdgeT>
bool SCGraph<NodeT, EdgeT>::isUp(Shortcut const& edge, EdgeType direction) const
{
	uint src_lvl = _node_levels[edge.src];
	uint tgt_lvl = _node_levels[edge.tgt];

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
