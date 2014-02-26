#include "defs.h"
#include "unit_tests.h"
#include "ch_constructor.h"
#include "parser.h"

#include <iostream>
#include <getopt.h>
#include <string>

using namespace chc;

void printHelp()
{
	Print("Usage: ./ch_constructor [ARGUMENTS]");
	Print("Mandatory arguments are:");
	Print("  -i, --infile <path>        Read graph from <path>");
	Print("Optional arguments are:");
	Print("  -f, --informat <format>    Expects infile in <format>");
	Print("  -o, --outfile <path>       Write graph to <path>");
	Print("  -g, --outformat <format>   Writes outfile in <format>");
	Print("  -t, --threads <number>     Number of threads to use in the calculations");
}

int main(int argc, char* argv[])
{
	/*
	 * Containers for arguments.
	 */

	std::string infile("");
	FileFormat informat(FMI);
	std::string outfile("ch_out.graph");
	FileFormat outformat(FMI_CH);
	uint nr_of_threads(1);

	/*
	 * Getopt argument parsing.
	 */

	const struct option longopts[] = {
		{"help",	no_argument,        0, 'h'},
		{"infile",	required_argument,  0, 'i'},
		{"informat",	required_argument,  0, 'f'},
		{"outfile",     required_argument,  0, 'o'},
		{"outformat",   required_argument,  0, 'g'},
		{"threads",	required_argument,  0, 't'},
		{0,0,0,0},
	};

	std::stringstream ss;
	int index(0);
	int iarg(0);
	opterr = 1;

	while((iarg = getopt_long(argc, argv, "hti:f:o:g:t:", longopts, &index)) != -1) {
		ss.clear();

		switch (iarg) {
			case 'h':
				printHelp();
				return 0;
				break;
			case 'i':
				infile = optarg;
				break;
			case 'f':
				informat = toFileFormat(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'g':
				outformat = toFileFormat(optarg);
				break;
			case 't':
				ss << optarg;
				ss >> nr_of_threads;
				break;
			default:
				printHelp();
				return 1;
				break;
		}
	}

	if (infile == "") {
		std::cerr << "No input file specified! Exiting." << std::endl;
		Print("Use ./ch_constructor --help to print the usage.");
		return 1;
	}

	/*
	 * CH
	 */

	typedef CHNode<Node> LvlNode;
	typedef CHEdge<Edge> Shortcut;
	typedef SCGraph<Node, Edge> CHGraph;

	/* Read graph */
	CHGraph g;
	Parser::InData<LvlNode,Shortcut> data;
	Parser::read<LvlNode,Shortcut>(data, infile, informat);
	g.init<EdgeSortSrc<Edge>, EdgeSortTgt<Edge> >(data);

	/* Build CH */
	CHConstructor<Node, Edge> chc(g, nr_of_threads);
	std::list<NodeID> all_nodes;
	for (uint i(0); i<g.getNrOfNodes(); i++) {
		all_nodes.push_back(i);
	}
	chc.quick_contract(all_nodes, 4, 5);
	chc.contract(all_nodes);
	chc.getCHGraph();

	/* Export */
	Parser::write<LvlNode,Shortcut>(g.getData(), outfile, outformat);

	return 0;
}
