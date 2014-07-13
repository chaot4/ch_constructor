#include "file_formats.h"

#include <random>
#include <sstream>

namespace chc {
	namespace {
		template<typename Callable>
		auto readLine(std::istream& is, Callable&& callable) -> decltype(callable(is)) {
			if (is.eof()) {
				std::cerr << "FATAL_ERROR: end of file\n";
				std::abort();
			}
#if 0
			/* "nice" way; parse the next non-empty line through a seperate stringstream */
			std::string line;
			do {
				std::getline(is, line);
			} while (is && line.empty());
			std::istringstream sis;
			std::cerr << "Reading line: '" << line << "'\n";
			sis.str(line);
			return callable(static_cast<std::istream&>(sis));
#else
			/* only make sure there is a newline (or EOF) after the record */
			auto r = callable(is);
			auto t = is.peek();
			if (is.fail()) {
				std::cerr << "FATAL_ERROR: istream failed\n";
				std::abort();
			}
			if ('\n' != t && std::istream::traits_type::eof() != t) {
				std::cerr << "Couldn't find new line after record\n";
				std::abort();
			}
			return r;
#endif
		}
	}

	FileFormat toFileFormat(std::string const& format)
	{
		if (format == "STD") {
			return STD;
		}
		else if (format == "SIMPLE") {
			return SIMPLE;
		}
		else if (format == "FMI") {
			return FMI;
		}
		else if (format == "FMI_CH") {
			return FMI_CH;
		}
		else {
			std::cerr << "Unknown fileformat: " << format << "\n";
		}

		return FMI;
	}

	std::string toString(FileFormat format)
	{
		if (format == STD) {
			return "STD";
		}
		else if (format == SIMPLE) {
			return "SIMPLE";
		}
		else if (format == FMI) {
			return "FMI";
		}
		else if (format == FMI_CH) {
			return "FMI_CH";
		}
		else {
			std::cerr << "Unknown fileformat: " << format << "\n";
		}

		return "FMI";
	}

	template<>
	void text_writeNode<OSMNode>(std::ostream& os, OSMNode const& node)
	{
		os << node.id << " " << node.osm_id << " " << node.lat << " "
			<< node.lon << " " << node.elev << "\n";
	}

	template<>
	OSMNode text_readNode<OSMNode>(std::istream& is, NodeID node_id)
	{
		return readLine(is, [node_id](std::istream& is) {
			OSMNode node;
			is >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;
			if (node_id != c::NO_NID && node.id != node_id) {
				std::cerr << "FATAL_ERROR: Invalid node id " << node.id << " at index " << node_id << ". Exiting\n";
				text_writeNode(std::cerr, node);
				std::abort();
			}
			return node;
		});
	}

	template<>
	void text_writeNode<GeoNode>(std::ostream& os, GeoNode const& node)
	{
		os << node.lat << " " << node.lon << " " << node.elev << "\n";
	}

	template<>
	GeoNode text_readNode<GeoNode>(std::istream& is, NodeID node_id)
	{
		return readLine(is, [node_id](std::istream& is) {
			GeoNode node;
			node.id = node_id;
			is >> node.lat >> node.lon >> node.elev;
			return node;
		});
	}

	template<>
	void text_writeEdge<OSMEdge>(std::ostream& os, OSMEdge const& edge)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << " "
			<< edge.type << " " << edge.speed << "\n";
	}

	template<>
	OSMEdge text_readEdge<OSMEdge>(std::istream& is, EdgeID edge_id)
	{
		return readLine(is, [edge_id](std::istream& is) {
			OSMEdge edge;
			is >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;
			edge.id = edge_id;
			return edge;
		});
	}

	template<>
	void text_writeEdge<Edge>(std::ostream& os, Edge const& edge)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << "\n";
	}

	template<>
	Edge text_readEdge<Edge>(std::istream& is, EdgeID edge_id)
	{
		return readLine(is, [edge_id](std::istream& is) {
			Edge edge;
			is >> edge.src >> edge.tgt >> edge.dist;
			edge.id = edge_id;
			return edge;
		});
	}


	namespace FormatSTD {
		void Reader_impl::readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges)
		{
			is >> estimated_nr_nodes >> estimated_nr_edges;
		}

		auto Reader_impl::readNode(NodeID node_id) -> node_type
		{
			return text_readNode<node_type>(is, node_id);
		}

		auto Reader_impl::readEdge(EdgeID edge_id) -> edge_type
		{
			return text_readEdge<edge_type>(is, edge_id);
		}

		void Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges)
		{
			os << nr_of_nodes << "\n";
			os << nr_of_edges << "\n";
		}

		void Writer_impl::writeNode(node_type const& out, NodeID node_id)
		{
			if (node_id != out.id) {
				std::cerr << "FATAL_ERROR: Invalid node id " << out.id << " at index " << node_id << ". Exiting\n";
				std::abort();
			}
			text_writeNode<node_type>(os, out);
		}

		void Writer_impl::writeEdge(edge_type const& out, EdgeID)
		{
			text_writeEdge<edge_type>(os, out);
		}
	}


	namespace FormatSimple {
		void Reader_impl::readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges)
		{
			is >> estimated_nr_nodes >> estimated_nr_edges;
		}

		auto Reader_impl::readNode(NodeID node_id) -> node_type
		{
			return text_readNode<node_type>(is, node_id);
		}

		auto Reader_impl::readEdge(EdgeID edge_id) -> edge_type
		{
			return text_readEdge<edge_type>(is, edge_id);
		}

		void Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges)
		{
			os << nr_of_nodes << "\n";
			os << nr_of_edges << "\n";
		}

		void Writer_impl::writeNode(node_type const& out, NodeID)
		{
			text_writeNode<node_type>(os, out);
		}

		void Writer_impl::writeEdge(edge_type const& out, EdgeID)
		{
			text_writeEdge<edge_type>(os, out);
		}
	}



	void FormatFMI::Reader_impl::readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges)
	{
		char c;
		is.get(c);
		// Note that this loop also reads the first character after
		// all the comments are over.
		while (c == '#') {
			is.ignore(1024, '\n');
			is.get(c);
		}

		is >> estimated_nr_nodes >> estimated_nr_edges;
	}

	/* Generate random id */
	static std::string random_id(unsigned int len)
	{
		static const char hex[17] = "0123456789abcdef";

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, 15);

		std::string s; s.resize(len);
		for (unsigned int i = 0; i < len; ++i) {
			s[i] = hex[dist(gen)];
		}
		return s;
	}

	void FormatFMI_CH::Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges)
	{
		/* Write header */
		os << "# Id : " << random_id(32) << "\n";
		os << "# Timestamp : " << time(nullptr) << "\n";
		os << "# Type: maxspeed" << "\n";
		os << "# Revision: 1" << "\n";
		os << "\n";

		os << nr_of_nodes << "\n";
		os << nr_of_edges << "\n";
	}
}
