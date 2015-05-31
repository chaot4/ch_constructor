#pragma once

#include "defs.h"
#include "nodes_and_edges.h"

#include <vector>

namespace chc
{

/*
 * Interface of a Prioritizer used in the CHConstructor.
 */
class Prioritizer
{
	public:
		/*
		 * Initialize the prioritizer list with <node_ids>.
		 */
		virtual void init(std::vector<NodeID>& node_ids) = 0;

		/*
		 * Removes <node_ids> from the list of nodes to be prioritized.
		 */
		virtual void remove(std::vector<bool> const& to_remove) = 0;

		/*
		 * Returns vector of nodes containing the ones to be contracted next.
		 *
		 * ATTENTION: The returned vector of nodes has to contain an independent set!
		 */
		virtual std::vector<NodeID> getNextNodes() = 0;

		/*
		 * Returns all the nodes still to be contracted.
		 */
		virtual std::vector<NodeID> const& getRemainingNodes() = 0;

		/*
		 * True iff there are still nodes left to contract.
		 */
		virtual bool hasNodesLeft() = 0;
};

}
