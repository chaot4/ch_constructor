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
		using BaseGraph::_out_edges;
		using BaseGraph::_in_edges;

		std::vector<Shortcut> _edges_dump;

	public:
		SCGraph() : BaseGraph() {}

		void restructure(std::vector<bool> const& deleted,
				std::vector<Shortcut>& new_shortcuts);
		void buildCHGraph();
};

template <typename Node, typename Edge>
void SCGraph<Node, Edge>::restructure(std::vector<bool> const& deleted,
				std::vector<Shortcut>& new_shortcuts)
{
	std::vector<Shortcut> new_edge_vec;
	new_edge_vec.reserve(_out_edges.size() + new_shortcuts.size());

	std::sort(new_shortcuts.begin(), new_shortcuts.end(), EdgeSortSrc<Shortcut>());

	/* Manually merge the new_shortcuts and _out_edges vector. */
	uint j(0);
	for (uint i(0); i<_out_edges.size(); i++) {

		Shortcut const& edge(_out_edges[i]);
		/* edge greater */
		while (j < new_shortcuts.size() && new_shortcuts[j] < edge) {

			Shortcut const& new_sc(new_shortcuts[j]);
			if (!deleted[new_sc.src] && !deleted[new_sc.tgt]) {
				new_edge_vec.push_back(new_sc);
			}
			else {
				_edges_dump.push_back(new_sc);
			}
			j++;
		}

		/* edge equal */
		while (j < new_shortcuts.size() && new_shortcuts[j] == edge) {
			j++;
		}

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

		Shortcut const& new_sc(new_shortcuts[j]);

		if (!deleted[new_sc.src] && !deleted[new_sc.tgt]) {
			new_edge_vec.push_back(new_sc);
		}
		else {
			_edges_dump.push_back(new_sc);
		}

		j++;
	}

	// TODO manually merge _in_edges. Is this really better?

	_out_edges.swap(new_edge_vec);
	_in_edges.assign(_out_edges.begin(), _out_edges.end());
	BaseGraph::template sortInEdges<EdgeSortTgt<Edge> >();
	BaseGraph::initOffsets();
	BaseGraph::initIdToIndex();

	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), EdgeSortSrc<Edge>()));
	assert(std::is_sorted(_in_edges.begin(), _in_edges.end(), EdgeSortTgt<Edge>()));
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

	assert(std::is_sorted(_out_edges.begin(), _out_edges.end(), EdgeSortSrc<Edge>()));
	assert(std::is_sorted(_in_edges.begin(), _in_edges.end(), EdgeSortTgt<Edge>()));
}

#endif
