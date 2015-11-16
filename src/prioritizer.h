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
		virtual ~Prioritizer() { };

		/*
		 * Initialize the list of all the nodes to be prioritized with <node_ids>.
		 */
		virtual void init(std::vector<NodeID>& node_ids) = 0;

		/*
		 * Returns a vector of nodes containing the ones to be contracted next and
		 * then deletes them from the list of nodes to be prioritized.
		 *
		 * ATTENTION: The returned vector of nodes has to contain an independent set!
		 */
		virtual std::vector<NodeID> extractNextNodes() = 0;

		/*
		 * True iff there are still nodes left to prioritize/contract.
		 */
		virtual bool hasNodesLeft() = 0;
};

}
