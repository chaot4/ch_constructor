#pragma once

#include "prioritizer.h"

#include <memory>

namespace chc
{

/*
 * All the Prioritizers go here.
 */

/*
 * Example of usage of the Prioritizer Interface.
 */
template <typename GraphT>
class My_Prioritizer : public Prioritizer
{
	private:
		GraphT const& _base_graph;
		std::vector<NodeID> _prio_vec;

	public:
		My_Prioritizer(GraphT const& base_graph) : _base_graph(base_graph) { }
		void init(std::vector<NodeID>& node_ids);
		void remove(std::vector<bool> const& to_remove);
		std::vector<NodeID> getNextNodes();
		std::vector<NodeID> const& getRemainingNodes();
		bool hasNodesLeft();
};

template <typename GraphT>
void My_Prioritizer<GraphT>::init(std::vector<NodeID>& node_ids)
{
	_prio_vec = std::move(node_ids);
}

template <typename GraphT>
void My_Prioritizer<GraphT>::remove(std::vector<bool> const& to_remove)
{
	size_t remaining_nodes(_prio_vec.size());
	for (size_t i(0); i<_prio_vec.size(); i++) {
		NodeID node(_prio_vec[i]);
		if (to_remove[node]) {
			remaining_nodes--;
			_prio_vec[i] = _prio_vec[remaining_nodes];
			_prio_vec[remaining_nodes] = node;
		}
	}

	_prio_vec.resize(remaining_nodes);
}

template <typename GraphT>
std::vector<NodeID> const& My_Prioritizer<GraphT>::getRemainingNodes()
{
	return _prio_vec;
}

template <typename GraphT>
std::vector<NodeID> My_Prioritizer<GraphT>::getNextNodes()
{
	assert(!_prio_vec.empty());
	return std::vector<NodeID>(1, _prio_vec.back());
}

template <typename GraphT>
bool My_Prioritizer<GraphT>::hasNodesLeft()
{
	return !_prio_vec.empty();
}

/*
 * New Prioritizers have to be included into the enum and the createPrioritizer function.
 */

enum class PrioritizerType { NONE = 0, MY };

PrioritizerType toPrioritizerType(std::string const& type)
{
	if (type == "MY") {
		return PrioritizerType::MY;
	}
	else if (type == "NONE") {
		return PrioritizerType::NONE;
	}
	else {
		std::cerr << "Unknown prioritizer type: " << type << "\n";
	}

	return PrioritizerType::MY;
}

template <class GraphT>
std::unique_ptr<Prioritizer> createPrioritizer(PrioritizerType prioritizer_type, GraphT const& graph)
{
	switch (prioritizer_type) {
	case PrioritizerType::MY:
		return std::unique_ptr<Prioritizer>(new My_Prioritizer<GraphT>(graph));
	case PrioritizerType::NONE:
		return nullptr;
	}

	return nullptr;
}

}
