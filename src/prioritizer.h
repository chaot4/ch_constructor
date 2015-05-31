#pragma once

#include "defs.h"
#include "nodes_and_edges.h"

#include <vector>

namespace chc
{

/*
 * Interface of a Prioritizer used in the CHConstructor.
 */
template <template <typename, typename...> class Container>
class Prioritizer
{
	protected:
		typedef Container<NodeID> container_type;
		container_type _prio_list;

	public:
		/*
		 * Initialize the prioritizer list with <node_ids>.
		 */
		virtual void init(container_type& node_ids) = 0;

		/*
		 * Removes <node_ids> from the list of nodes to be prioritized.
		 */
		virtual void remove(std::vector<bool> to_remove) = 0;

		/*
		 * Returns vector of nodes containing the ones to be contracted next.
		 *
		 * ATTENTION: The returned vector of nodes has to contain an independent set!
		 */
		virtual std::vector<NodeID> getNextNodes() = 0;

		/*
		 * True iff there are still nodes left to contract.
		 */
		virtual bool hasNodesLeft() = 0;
};

}
