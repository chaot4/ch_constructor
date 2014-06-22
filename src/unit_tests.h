#ifndef _UNIT_TESTS_H
#define _UNIT_TESTS_H

#include "nodes_and_edges.h"
#include "graph.h"
#include "file_formats.h"
#include "chgraph.h"
#include "ch_constructor.h"
#include "dijkstra.h"

#include <map>
#include <iostream>
#include <random>
#include <chrono>

namespace chc
{

#define Test(x)\
if (!(x)) {\
	std::cout << "TEST_FAILED: " << #x << std::endl;\
	exit(1);\
}\

namespace unit_tests
{
	void testAll();
}

}

#endif
