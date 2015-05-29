#pragma once

#include "defs.h"
#include "nodes_and_edges.h"
#include "chgraph.h"

#include <list>
#include <vector>

namespace chc
{

/*
 * Interface of a Prioritizer used in the CHConstructor.
 */
template <template <typename, typename...> class Container = std::list>
class Prioritizer
{
	protected:
		typedef Container<NodeID> container_type;
		container_type _prio_list;

	public:
		/*
		 * Initialize the prioritizer list with <node_id_list>.
		 *
		 * ATTENTION: Steals the content of nodes!
		 */
		void init(container_type& node_id_list);

		/*
		 * Removes <node_id_list> from the list of nodes to be prioritized.
		 * This is a generic remove. It might be faster to implement
		 * ones own remove function in a derived class.
		 */
		virtual void remove(std::vector<bool> to_remove);

		/*
		 * Returns list of nodes in (descending) order of their priority.
		 */
		container_type const& getPriorityList();

		/*
		 * Reevaluate the priorities of the nodes and update the list accordingly.
		 * This function uses order_by_prio to achieve that.
		 */
		void update();

		/*
		 * Order <node_id_list> by priorities (descending).
		 */
		virtual void order_by_prio(container_type& node_id_list) = 0;
};

template <template <typename, typename...> class Container>
void Prioritizer<Container>::init(container_type& node_id_list)
{
	_prio_list = std::move(node_id_list);
}

template <template <typename, typename...> class Container>
void Prioritizer<Container>::remove(std::vector<bool> to_remove)
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

template <template <typename, typename...> class Container>
auto Prioritizer<Container>::getPriorityList() -> container_type const& 
{
	return _prio_list;
}

template <template <typename, typename...> class Container>
void Prioritizer<Container>::update()
{
	order_by_prio(_prio_list);
}

/*
 * Example of usage of the Prioritizer Interface.
 */
template <typename NodeT, typename EdgeT>
class My_Prioritizer : public Prioritizer<std::list>
{
	private:
		typedef My_Prioritizer::container_type container_type;
		typedef CHGraph<NodeT, EdgeT> CHGraphT;

		CHGraphT const& _base_graph;

	public:
		My_Prioritizer(CHGraphT const& base_graph) : _base_graph(base_graph) { }
		void order_by_prio(container_type& node_id_list);
};

template <typename NodeT, typename EdgeT>
void My_Prioritizer<NodeT, EdgeT>::order_by_prio(container_type& node_id_list)
{
	(void) node_id_list;
	// TODO
}

}
