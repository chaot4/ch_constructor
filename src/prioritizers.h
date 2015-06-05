#pragma once

#include "prioritizer.h"
#include "nodes_and_edges.h"

#include <memory>

namespace chc
{

namespace unit_tests
{
	void testPrioritizers();
}

/*
 * All the Prioritizers go here.
 */

/*
 * Example of usage of the Prioritizer Interface.
 */
template <class GraphT>
class OneByOnePrioritizer : public Prioritizer
{
	private:
		GraphT const& _base_graph;
		std::vector<NodeID> _prio_vec;

		std::vector<NodeID> _chooseIndependentSet();
		void _remove(std::vector<NodeID> const& nodes);
	public:
		OneByOnePrioritizer(GraphT const& base_graph) : _base_graph(base_graph) { }
		void init(std::vector<NodeID>& node_ids); // steals the data from node_ids
		std::vector<NodeID> extractNextNodes();
		bool hasNodesLeft();

		friend void unit_tests::testPrioritizers();
};

template <class GraphT>
void OneByOnePrioritizer<GraphT>::_remove(std::vector<NodeID> const& nodes)
{
	std::vector<bool> to_remove(_base_graph.getNrOfNodes(), false);
	for (auto node: nodes) {
		to_remove[node] = true;
	}

	size_t remaining_nodes(_prio_vec.size());
	size_t i(0);
	while (i < remaining_nodes) {
		NodeID node(_prio_vec[i]);
		if (to_remove[node]) {
			remaining_nodes--;
			_prio_vec[i] = _prio_vec[remaining_nodes];
			_prio_vec[remaining_nodes] = node;
		}
		else {
			i++;
		}
	}

	_prio_vec.resize(remaining_nodes);
}

template <class GraphT>
std::vector<NodeID> OneByOnePrioritizer<GraphT>::_chooseIndependentSet() {
	return std::vector<NodeID>(1, _prio_vec.back());
}

template <class GraphT>
void OneByOnePrioritizer<GraphT>::init(std::vector<NodeID>& node_ids)
{
	_prio_vec = std::move(node_ids);
}

template <class GraphT>
std::vector<NodeID> OneByOnePrioritizer<GraphT>::extractNextNodes()
{
	assert(!_prio_vec.empty());

	auto next_nodes(_chooseIndependentSet());
	_remove(next_nodes);

	return next_nodes;
}

template <class GraphT>
bool OneByOnePrioritizer<GraphT>::hasNodesLeft()
{
	return !_prio_vec.empty();
}

/*
 * Prioritizer that prioritizes by edge difference with a greedy hitting set.
 */
template <class GraphT, class CHConstructorT>
class EdgeDiffPrioritizer : public Prioritizer
{
	private:
		struct CompInOutProduct;

		GraphT const& _base_graph;
		CHConstructorT const& _chc;
		std::vector<NodeID> _prio_vec;

		std::vector<NodeID> _chooseIndependentSet();
		void _remove(std::vector<NodeID> const& nodes);
	public:
		EdgeDiffPrioritizer(GraphT const& base_graph, CHConstructorT const& chc)
			: _base_graph(base_graph), _chc(chc) { }
		void init(std::vector<NodeID>& node_ids); // steals the data from node_ids
		std::vector<NodeID> extractNextNodes();
		bool hasNodesLeft();

		friend void unit_tests::testPrioritizers();
};

template <class GraphT, class CHConstructorT>
struct EdgeDiffPrioritizer<GraphT, CHConstructorT>::CompInOutProduct
{
	GraphT const& g;

	CompInOutProduct(GraphT const& g)
		: g(g) {}

	bool operator()(NodeID node1, NodeID node2) const
	{
		uint edge_product1(g.getNrOfEdges(node1, EdgeType::IN)
				* g.getNrOfEdges(node1, EdgeType::OUT));
		uint edge_product2(g.getNrOfEdges(node2, EdgeType::IN)
				* g.getNrOfEdges(node2, EdgeType::OUT));

		return edge_product1 < edge_product2;
	}
};

template <class GraphT, class CHConstructorT>
void EdgeDiffPrioritizer<GraphT, CHConstructorT>::_remove(std::vector<NodeID> const& nodes)
{
	std::vector<bool> to_remove(_base_graph.getNrOfNodes(), false);
	for (auto node: nodes) {
		to_remove[node] = true;
	}

	size_t remaining_nodes(_prio_vec.size());
	size_t i(0);
	while (i < remaining_nodes) {
		NodeID node(_prio_vec[i]);
		if (to_remove[node]) {
			remaining_nodes--;
			_prio_vec[i] = _prio_vec[remaining_nodes];
			_prio_vec[remaining_nodes] = node;
		}
		else {
			i++;
		}
	}

	_prio_vec.resize(remaining_nodes);
}

template <class GraphT, class CHConstructorT>
std::vector<NodeID> EdgeDiffPrioritizer<GraphT, CHConstructorT>::_chooseIndependentSet() {
	std::sort(_prio_vec.begin(), _prio_vec.end(), CompInOutProduct(_base_graph));
	auto independent_set(_chc.calcIndependentSet(_prio_vec));
	auto edge_diffs(_chc.calcEdgeDiffs(independent_set));

	double edge_diff_mean(0);
	for (size_t i(0); i<edge_diffs.size(); i++) {
		edge_diff_mean += edge_diffs[i];
	}
	edge_diff_mean /= independent_set.size();

	std::vector<NodeID> low_edge_diff_nodes;
	for (size_t i(0); i<independent_set.size(); i++) {
		if (edge_diffs[i] <= edge_diff_mean) {
			NodeID node(independent_set[i]);
			low_edge_diff_nodes.push_back(node);
		}
	}

	return low_edge_diff_nodes;
}

template <class GraphT, class CHConstructorT>
void EdgeDiffPrioritizer<GraphT, CHConstructorT>::init(std::vector<NodeID>& node_ids)
{
	_prio_vec = std::move(node_ids);
}

template <class GraphT, class CHConstructorT>
std::vector<NodeID> EdgeDiffPrioritizer<GraphT, CHConstructorT>::extractNextNodes()
{
	assert(!_prio_vec.empty());

	auto next_nodes(_chooseIndependentSet());
	_remove(next_nodes);

	return next_nodes;
}

template <class GraphT, class CHConstructorT>
bool EdgeDiffPrioritizer<GraphT, CHConstructorT>::hasNodesLeft()
{
	return !_prio_vec.empty();
}

/*
 * New Prioritizers have to be included into the enum and the createPrioritizer function.
 */

enum class PrioritizerType { NONE = 0, ONE_BY_ONE, EDGE_DIFF };
static constexpr PrioritizerType LastPrioritizerType = PrioritizerType::EDGE_DIFF;

PrioritizerType toPrioritizerType(std::string const& type)
{
	if (type == "NONE") {
		return PrioritizerType::NONE;
	}
	else if (type == "ONE_BY_ONE") {
		return PrioritizerType::ONE_BY_ONE;
	}
	else if (type == "EDGE_DIFF") {
		return PrioritizerType::EDGE_DIFF;
	}
	else {
		std::cerr << "Unknown prioritizer type: " << type << "\n";
	}

	return PrioritizerType::NONE;
}

std::string to_string(PrioritizerType type)
{
	switch (type) {
	case PrioritizerType::NONE:
		return "NONE";
	case PrioritizerType::ONE_BY_ONE:
		return "ONE_BY_ONE";
	case PrioritizerType::EDGE_DIFF:
		return "EDGE_DIFF";
	}

	std::cerr << "Unknown prioritizer type: " << static_cast<int>(type) << "\n";
	return "NONE";
}

template <class GraphT, class CHConstructorT>
std::unique_ptr<Prioritizer> createPrioritizer(PrioritizerType prioritizer_type, GraphT const& graph,
		CHConstructorT const& chc)
{
	switch (prioritizer_type) {
	case PrioritizerType::NONE:
		return nullptr;
	case PrioritizerType::ONE_BY_ONE:
		return std::unique_ptr<Prioritizer>(new OneByOnePrioritizer<GraphT>(graph));
	case PrioritizerType::EDGE_DIFF:
		return std::unique_ptr<Prioritizer>(new EdgeDiffPrioritizer<GraphT, CHConstructorT>(graph, chc));
	}

	return nullptr;
}

}
