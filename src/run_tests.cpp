#include "defs.h"
#include "unit_tests.h"
#include "ch_constructor.h"
#include "parser.h"

#include <iostream>
#include <getopt.h>
#include <string>

using namespace chc;

int main(int argc, char* argv[])
{
	unit_tests::testAll();
	return 0;
}
