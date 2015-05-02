#include "defs.h"
#include "unit_tests.h"
#include "ch_constructor.h"
#include "file_formats.h"
#include "track_time.h"

#include <getopt.h>

using namespace chc;
using namespace std::chrono;

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
	TrackTime tt;

	template<typename NodeT, typename EdgeT>
	void operator()(GraphInData<NodeT, CHEdge<EdgeT>>&& data) {
		tt.track("reading input");

		/* Read graph */
		SCGraph<NodeT, EdgeT> g;
		g.init(std::move(data));
		tt.track("loading graph");

		/* Build CH */
		CHConstructor<NodeT, EdgeT> chc(g, nr_of_threads);
		std::list<NodeID> all_nodes;
		for (uint i(0); i<g.getNrOfNodes(); i++) {
			all_nodes.push_back(i);
		}
		chc.quick_contract(all_nodes, 4, 5);
		chc.contract(all_nodes);

		tt.track("contracting graph");

		auto exportData = g.exportData();
		tt.track("rebuliding graph");

		/* Export */
		writeCHGraphFile(outformat, outfile, std::move(exportData));
		tt.track("exporting graph", false);

		tt.summary();
	}
};

int main(int argc, char* argv[])
{
	/*
	 * Containers for arguments.
	 */

	std::string infile("");
	FileFormat informat(FileFormat::FMI);
	std::string outfile("ch_out.graph");
	FileFormat outformat(FileFormat::FMI_CH);
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

	readGraphForWriteFormat(outformat, informat, infile,
		BuildAndStoreCHGraph { outformat, outfile, nr_of_threads,
#ifndef NVERBOSE
			TrackTime(std::cout)
#else
			TrackTime()
#endif
		}
	);

	return 0;
}
