#pragma once

#include <list>

namespace chc
{

template <typename NodeT, typename EdgeT, typename Container = std::list<NodeT>>
class Prioritizer
{
	private:
		Container _nodes;
	public:
		virtual void init(Container const& nodes) = 0; // FIXME really Container?
		virtual void remove(Container const& nodes) = 0; // FIXME really Container?
		virtual Container const& getPriorityList() = 0;
		virtual void update() = 0;
};

}
