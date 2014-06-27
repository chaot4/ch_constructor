#include "defs.h"
#include "unit_tests.h"
#include "ch_constructor.h"
#include "file_formats.h"

#include <iostream>
#include <getopt.h>
#include <string>

using namespace chc;

void printHelp()
{
	std::cout
		<< "Usage: ./ch_constructor [ARGUMENTS]\n"
		<< "Mandatory arguments are:\n"
		<< "  -i, --infile <path>        Read graph from <path>\n"
		<< "Optional arguments are:\n"
		<< "  -f, --informat <format>    Expects infile in <format> (SIMPLE, STD, FMI - default FMI)\n"
		<< "  -o, --outfile <path>       Write graph to <path> (default: ch_out.graph)\n"
		<< "  -g, --outformat <format>   Writes outfile in <format> (SIMPLE, STD, FMI_CH - default FMI_CH)\n"
		<< "  -t, --threads <number>     Number of threads to use in the calculations (default: 1)\n";
}

struct BuildAndStoreCHGraph {
	FileFormat outformat;
	std::string outfile;
	uint nr_of_threads;

	template<typename NodeT, typename EdgeT>
	void operator()(GraphInData<NodeT, CHEdge<EdgeT>>&& data) {
		/* Read graph */
		SCGraph<NodeT, EdgeT> g;
		g.init(std::move(data));

		/* Build CH */
		CHConstructor<NodeT, EdgeT> chc(g, nr_of_threads);
		std::list<NodeID> all_nodes;
		for (uint i(0); i<g.getNrOfNodes(); i++) {
			all_nodes.push_back(i);
		}
		chc.quick_contract(all_nodes, 4, 5);
		chc.contract(all_nodes);
		chc.rebuildCompleteGraph();

		/* Export */
		writeCHGraphFile(outformat, outfile, g.getData());
	}
};

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

	int index(0);
	int iarg(0);
	opterr = 1;

	while((iarg = getopt_long(argc, argv, "hi:f:o:g:t:", longopts, &index)) != -1) {
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
				{
					size_t idx = 0; // index of first "non digit"
					nr_of_threads = std::stoi(optarg, &idx);
					if ('\0' != optarg[idx] || nr_of_threads <= 0) {
						std::cerr << "Invalid thread count: '" << optarg << "'\n";
						return 1;
					}
				}
				break;
			default:
				printHelp();
				return 1;
				break;
		}
	}

	if (infile == "") {
		std::cerr << "No input file specified! Exiting.\n";
		std::cerr << "Use ./ch_constructor --help to print the usage.\n";
		return 1;
	}

	Print("Using " << nr_of_threads << " threads.");

	readGraphForWriteFormat(outformat, informat, infile, BuildAndStoreCHGraph { outformat, outfile, nr_of_threads });

	return 0;
}
