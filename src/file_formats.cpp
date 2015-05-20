#include "file_formats.h"

#include <random>
#include <sstream>
#include <string>

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
			return FileFormat::STD;
		}
		else if (format == "SIMPLE") {
			return FileFormat::SIMPLE;
		}
		else if (format == "FMI") {
			return FileFormat::FMI;
		}
		else if (format == "FMI_DIST") {
			return FileFormat::FMI_DIST;
		}
		else if (format == "FMI_EUCL") {
			return FileFormat::FMI_EUCL;
		}
		else if (format == "FMI_CH") {
			return FileFormat::FMI_CH;
		}
		else if (format == "FMI_EUCL_CH") {
			return FileFormat::FMI_EUCL_CH;
		}
		else if (format == "STEFAN_CH") {
			return FileFormat::STEFAN_CH;
		}
		else {
			std::cerr << "Unknown fileformat: " << format << "\n";
		}

		return FileFormat::FMI;
	}

	std::string to_string(FileFormat format)
	{
		switch (format) {
		case FileFormat::STD:
			return "STD";
		case FileFormat::SIMPLE:
			return "SIMPLE";
		case FileFormat::FMI:
			return "FMI";
		case FileFormat::FMI_DIST:
			return "FMI_DIST";
		case FileFormat::FMI_EUCL:
			return "FMI_EUCL";
		case FileFormat::FMI_CH:
			return "FMI_CH";
		case FileFormat::FMI_EUCL_CH:
			return "FMI_EUCL_CH";
		case FileFormat::STEFAN_CH:
			return "STEFAN_CH";
		}

		std::cerr << "Unknown fileformat: " << static_cast<int>(format) << "\n";
		return "FMI";
	}

	std::vector<FileFormat> getAllFileFormats() {
		std::vector<FileFormat> result;
		size_t const last = from_enum(LastFileFormat);
		for (size_t i = 0; i <= last; ++i) {
			result.emplace_back(static_cast<FileFormat>(i));
		}
		return result;
	}

	std::string getAllFileFormatsString() {
		std::stringstream s;
		size_t const last = from_enum(LastFileFormat);
		s << to_string(static_cast<FileFormat>(0));
		for (size_t i = 1; i <= last; ++i) {
			s << ", " << to_string(static_cast<FileFormat>(i));
		}
		return s.str();
	}

	static std::string random_id(unsigned int len)
	{
		static const char hex[17] = "0123456789abcdef";

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, 15);

		std::string s; s.resize(len);
		for (auto& c: s) {
			c = hex[dist(gen)];
		}
		return s;
	}

	void calcTimeMetric(OSMEdge& edge)
	{
		/* Stolen from ToureNPlaner */
		if (edge.speed <= 0){
			switch (edge.type) {
				//motorway
				case 1:
					edge.dist = (edge.dist * 1.3) / 1.3;
					break;
					//motorway link
				case 2:
					edge.dist = (edge.dist * 1.3) / 1.0;
					break;
					//primary
				case 3:
					edge.dist = (edge.dist * 1.3) / 0.7;
					break;
					//primary link
				case 4:
					edge.dist = (edge.dist * 1.3) / 0.7;
					break;
					//secondary
				case 5:
					edge.dist = (edge.dist * 1.3) / 0.65;
					break;
					//secondary link
				case 6:
					edge.dist = (edge.dist * 1.3) / 0.65;
					break;
					//tertiary
				case 7:
					edge.dist = (edge.dist * 1.3) / 0.6;
					break;
					//tertiary link
				case 8:
					edge.dist = (edge.dist * 1.3) / 0.6;
					break;
					//trunk
				case 9:
					edge.dist = (edge.dist * 1.3) / 0.8;
					break;
					//trunk link
				case 10:
					edge.dist = (edge.dist * 1.3) / 0.8;
					break;
					//unclassified
				case 11:
					edge.dist = (edge.dist * 1.3) / 0.25;
					break;
					//residential
				case 12:
					edge.dist = (edge.dist * 1.3) / 0.45;
					break;
					//living street
				case 13:
					edge.dist = (edge.dist * 1.3) / 0.3;
					break;
					//road
				case 14:
					edge.dist = (edge.dist * 1.3) / 0.25;
					break;
					//service
				case 15:
					edge.dist = (edge.dist * 1.3) / 0.3;
					break;
					//turning circle
				case 16:
					edge.dist = (edge.dist * 1.3) / 0.3;
					break;
				default:
					edge.dist = (edge.dist * 1.3) / 0.5;
			}
		} else {
			edge.dist = (edge.dist * 1.3) / (((edge.speed > 130) ? 130.0 : (double) edge.speed)/100.0);
		}
	}

	template<>
	void text_writeNode<OSMNode>(std::ostream& os, OSMNode const& node)
	{
		os << node.id << " " << node.osm_id << " " << node.lat << " "
			<< node.lon << " " << node.elev << "\n";
	}

	template<>
	void text_writeNode<CHNode<OSMNode>>(std::ostream& os, CHNode<OSMNode> const& node)
	{
		os << node.id << " " << node.osm_id << " " << node.lat << " "
			<< node.lon << " " << node.elev << " " << node.lvl << "\n";
	}

	template<>
	void text_writeNode<CHNode<StefanNode>>(std::ostream& os, CHNode<StefanNode> const& node)
	{
		os << node.lon << " " << node.lat << " " <<  node.lvl << " " << node.osm_id << "\n";
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
			std::make_signed<decltype(edge.dist)>::type signed_dist;

			is >> edge.src >> edge.tgt >> signed_dist >> edge.type >> edge.speed;
			if (signed_dist >= 0) {
				edge.id = edge_id;
				edge.dist = signed_dist;
			} else {
				// mark as invalid edge with 0 length
				edge.id = c::NO_EID;
				edge.dist = 0;
			}
			calcTimeMetric(edge);
			return edge;
		});
	}

	template<>
	EuclOSMEdge text_readEdge<EuclOSMEdge>(std::istream& is, EdgeID edge_id)
	{
		return readLine(is, [edge_id](std::istream& is) {
			EuclOSMEdge edge;
			std::make_signed<decltype(edge.dist)>::type signed_dist;

			is >> edge.src >> edge.tgt >> signed_dist >> edge.type >> edge.speed;
			if (signed_dist >= 0) {
				edge.id = edge_id;
				edge.dist = signed_dist;
			} else {
				// mark as invalid edge with 0 length
				edge.id = c::NO_EID;
				edge.dist = 0;
			}
			edge.eucl_dist = edge.dist;
			calcTimeMetric(edge);
			return edge;
		});
	}

	template<>
	OSMDistEdge text_readEdge<OSMDistEdge>(std::istream& is, EdgeID edge_id)
	{
		return readLine(is, [edge_id](std::istream& is) {
			OSMDistEdge edge;
			std::make_signed<decltype(edge.dist)>::type signed_dist;

			is >> edge.src >> edge.tgt >> signed_dist >> edge.type >> edge.speed;
			if (signed_dist >= 0) {
				edge.id = edge_id;
				edge.dist = signed_dist;
			} else {
				// mark as invalid edge with 0 length
				edge.id = c::NO_EID;
				edge.dist = 0;
			}
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
			std::make_signed<decltype(edge.dist)>::type signed_dist;

			is >> edge.src >> edge.tgt >> signed_dist;
			if (signed_dist >= 0) {
				edge.id = edge_id;
				edge.dist = signed_dist;
			} else {
				// mark as invalid edge with 0 length
				edge.id = c::NO_EID;
				edge.dist = 0;
			}
			return edge;
		});
	}

	template<>
	void text_writeEdge<CHEdge<OSMEdge>>(std::ostream& os, CHEdge<OSMEdge> const& edge)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << " "
			<< edge.type << " " << edge.speed << " "
			<< (edge.child_edge1 == c::NO_EID ? "-1" : std::to_string(edge.child_edge1)) << " "
			<< (edge.child_edge2 == c::NO_EID ? "-1" : std::to_string(edge.child_edge2)) << "\n";
	}

	template<>
	void text_writeEdge<CHEdge<EuclOSMEdge>>(std::ostream& os, CHEdge<EuclOSMEdge> const& edge)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << " "
			<< edge.type << " " << edge.eucl_dist << " "
			<< (edge.child_edge1 == c::NO_EID ? "-1" : std::to_string(edge.child_edge1)) << " "
			<< (edge.child_edge2 == c::NO_EID ? "-1" : std::to_string(edge.child_edge2)) << "\n";
	}

	template<>
	void text_writeEdge<CHEdge<StefanEdge>>(std::ostream& os, CHEdge<StefanEdge> const& edge)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << " "
			<< (edge.child_edge1 == c::NO_EID ? "-1" : std::to_string(edge.child_edge1)) << " "
			<< (edge.child_edge2 == c::NO_EID ? "-1" : std::to_string(edge.child_edge2)) << "\n";
	}


	namespace FormatSTD {
		void Reader_impl::readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges,
				Metadata& meta_data)
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

		Writer_impl::Writer_impl(std::ostream& os) : os(os) {
			os.precision(7);
			os << std::fixed;
		}

		void Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges, Metadata const& meta_data)
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
		void Reader_impl::readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges,
				Metadata& meta_data)
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

		Writer_impl::Writer_impl(std::ostream& os) : os(os) {
			os.precision(7);
			os << std::fixed;
		}

		void Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges, Metadata const& meta_data)
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

	namespace FormatFMI {
		void Reader_impl::readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges,
				Metadata& meta_data)
		{
			std::string line;
			std::getline(is, line);
			while (line != "") {
				std::stringstream ss(line);
				std::string hash;
				std::string key;
				std::string map;
				std::string colon;
				ss >> hash >> key >> colon >> map;

				if (hash != "#") {
					std::cout << "Error while parsing meta data: expected '#' instead of '"
						<< hash << "'\n";
				}
				if (colon != ":") {
					std::cout << "Error while parsing meta data: expected ':' instead of '"
						<< colon << "'\n";
				}

				meta_data[key] = map;

				std::getline(is, line);
			}

			is >> estimated_nr_nodes >> estimated_nr_edges;
		}

		Writer_impl::Writer_impl(std::ostream& os) : FormatSTD::Writer_impl(os) {
			os.precision(7);
			os << std::fixed;
		}

		void Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges, Metadata const& meta_data)
		{
			os << "# Type : graph" << "\n";
			os << "# Id : " << random_id(32) << "\n";
			os << "# Revision : 1" << "\n";
			os << "# Timestamp : " << time(nullptr) << "\n";
			os << "# Origin : ch_constructor" << "\n";
			for (auto const& meta_datum: meta_data) {
				os << "# Origin" << meta_datum.first << " : " << meta_datum.second << "\n";
			}
			os << "\n";

			os << nr_of_nodes << "\n";
			os << nr_of_edges << "\n";
		}
	}

	namespace FormatFMI_DIST {
		auto Reader_impl::readEdge(EdgeID edge_id) -> edge_type
		{
			return text_readEdge<edge_type>(is, edge_id);
		}
	}

	namespace FormatFMI_EUCL {
		auto Reader_impl::readEdge(EdgeID edge_id) -> edge_type
		{
			return text_readEdge<edge_type>(is, edge_id);
		}
	}

	namespace FormatFMI_CH {
		Writer_impl::Writer_impl(std::ostream& os) : FormatSTD::Writer_impl(os) {
			os.precision(7);
			os << std::fixed;
		}

		void Writer_impl::writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges, Metadata const& meta_data)
		{
			os << "# Type : chgraph" << "\n";
			os << "# Id : " << random_id(32) << "\n";
			os << "# Revision : 1" << "\n";
			os << "# Timestamp : " << time(nullptr) << "\n";
			os << "# Origin : ch_constructor" << "\n";
			for (auto const& meta_datum: meta_data) {
				os << "# Origin" << meta_datum.first << " : " << meta_datum.second << "\n";
			}
			os << "\n";

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

	namespace FormatFMI_EUCL_CH {
		void Writer_impl::writeEdge(edge_type const& out, EdgeID)
		{
			text_writeEdge<edge_type>(os, out);
		}
	}

	namespace FormatSTEFAN_CH {
		Writer_impl::Writer_impl(std::ostream& os) : FormatSTD::Writer_impl(os) {
			os.precision(7);
			os << std::fixed;
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
}
