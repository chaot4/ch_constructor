#pragma once

#include "prioritizer.h"
#include "chgraph.h"

#include <list>

namespace chc
{

/*
 * Example of usage of the Prioritizer Interface.
 */
template <typename NodeT, typename EdgeT>
class My_Prioritizer : public Prioritizer<std::list>
{
	private:
		typedef Prioritizer<std::list> Base_Prioritizer;
		using Base_Prioritizer::container_type;
		using Base_Prioritizer::_prio_list;

		typedef CHGraph<NodeT, EdgeT> CHGraphT;

		CHGraphT const& _base_graph;

	public:
		My_Prioritizer(CHGraphT const& base_graph) : _base_graph(base_graph) { }
		void init(container_type& node_ids);
		void remove(std::vector<bool> to_remove);
		std::vector<NodeID> getNextNodes();
		bool hasNodesLeft();
};

template <typename NodeT, typename EdgeT>
void My_Prioritizer<NodeT, EdgeT>::init(container_type& node_ids)
{
	_prio_list = std::move(node_ids);
}

template <typename NodeT, typename EdgeT>
bool My_Prioritizer<NodeT, EdgeT>::hasNodesLeft()
{
	return !_prio_list.empty();
}

template <typename NodeT, typename EdgeT>
void My_Prioritizer<NodeT, EdgeT>::remove(std::vector<bool> to_remove)
{
	auto it(_prio_list.begin());
	while (it != _prio_list.end()) {
		if (to_remove[*it]) {
			it = _prio_list.erase(it);
		}
		else {
			it++;
		}
	}
}

template <typename NodeT, typename EdgeT>
std::vector<NodeID> My_Prioritizer<NodeT, EdgeT>::getNextNodes()
{
	assert(!_prio_list.empty());
	return std::vector<NodeID>(1, _prio_list.back());
}

}
