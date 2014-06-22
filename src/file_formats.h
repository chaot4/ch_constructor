#ifndef _FILE_FORMATS_H
#define _FILE_FORMATS_H

#include "file_formats_helper.h"

namespace chc {
	// "default" text serialization of some nodes and edge types,
	// used in the the formats below
	template<typename NodeT>
	void text_writeNode(std::ostream& os, NodeT const& node);
	template<> void text_writeNode<OSMNode>(std::ostream& os, OSMNode const& node);
	template<> void text_writeNode<GeoNode>(std::ostream& os, GeoNode const& node);

	template<typename NodeT>
	NodeT text_readNode(std::istream& is, NodeID node_id = c::NO_NID);
	template<> OSMNode text_readNode<OSMNode>(std::istream& is, NodeID node_id);
	template<> GeoNode text_readNode<GeoNode>(std::istream& is, NodeID node_id);

	template<typename EdgeT>
	void text_writeEdge(std::ostream& os, EdgeT const& edge);
	template<> void text_writeEdge<OSMEdge>(std::ostream& os, OSMEdge const& edge);
	template<> void text_writeEdge<Edge>(std::ostream& os, Edge const& edge);

	template<typename EdgeT>
	EdgeT text_readEdge(std::istream& is, EdgeID edge_id = c::NO_EID);
	template<> OSMEdge text_readEdge<OSMEdge>(std::istream& is, EdgeID edge_id);
	template<> Edge text_readEdge<Edge>(std::istream& is, EdgeID edge_id);

	namespace FormatSTD
	{
		typedef OSMNode node_type;
		typedef OSMEdge edge_type;

		struct Reader_impl
		{
			Reader_impl(std::istream& is) : is(is) { }
			void readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges);
			node_type readNode(NodeID node_id);
			edge_type readEdge(EdgeID edge_id);
		protected:
			std::istream& is;
		};
		typedef SimpleReader<Reader_impl> Reader;

		struct Writer_impl
		{
			Writer_impl(std::ostream& os) : os(os) { }
			void writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges);
			void writeNode(node_type const& out, NodeID node_id);
			void writeEdge(edge_type const& out, EdgeID edge_id);
		protected:
			std::ostream& os;
		};
		typedef SimpleWriter<Writer_impl> Writer;
	}


	namespace FormatSimple
	{
		typedef GeoNode node_type;
		typedef Edge edge_type;

		struct Reader_impl
		{
			Reader_impl(std::istream& is) : is(is) { }
			void readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges);
			node_type readNode(NodeID node_id);
			edge_type readEdge(EdgeID edge_id);
		protected:
			std::istream& is;
		};
		typedef SimpleReader<Reader_impl> Reader;

		struct Writer_impl
		{
		public:
			Writer_impl(std::ostream& os) : os(os) { }
			void writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges);
			void writeNode(node_type const& out, NodeID node_id);
			void writeEdge(edge_type const& out, EdgeID edge_id);
		protected:
			std::ostream& os;
		};
		typedef SimpleWriter<Writer_impl> Writer;
	}

	/* only header is different from FormatSTD */
	namespace FormatFMI
	{
		typedef OSMNode node_type;
		typedef OSMEdge edge_type;

		struct Reader_impl : public FormatSTD::Reader_impl
		{
			Reader_impl(std::istream& is) : FormatSTD::Reader_impl(is) { }
			void readHeader(NodeID& estimated_nr_nodes, EdgeID& estimated_nr_edges);
		};
		typedef SimpleReader<Reader_impl> Reader;
	}
	namespace FormatFMI_CH
	{
		typedef OSMNode node_type;
		typedef OSMEdge edge_type;

		struct Writer_impl : public FormatSTD::Writer_impl
		{
		public:
			Writer_impl(std::ostream& os) : FormatSTD::Writer_impl(os) { }
			void writeHeader(NodeID nr_of_nodes, EdgeID nr_of_edges);
		};
		typedef SimpleWriter<Writer_impl> Writer;
	}



	enum FileFormat { STD, SIMPLE, FMI, FMI_CH };
	FileFormat toFileFormat(std::string const& format);

	template<typename Node, typename Edge>
	inline GraphInData<Node, Edge> readGraph(FileFormat format, std::string const& filename)
	{
		switch (format) {
		case STD:
			return FormatSTD::Reader::readGraph<Node, Edge>(filename);
		case SIMPLE:
			return FormatSimple::Reader::readGraph<Node, Edge>(filename);
		case FMI:
			return FormatFMI::Reader::readGraph<Node, Edge>(filename);
		case FMI_CH:
			break;
		}
		std::cerr << "Unknown input fileformat: " << format << std::endl;
		std::exit(1);
	}

	/* run callable with types from reader (but always with CHEdge<>) */
	template<typename Callable>
	inline void withReadGraph(FileFormat format, std::string const& filename, Callable&& callable)
	{
		switch (format) {
		case STD:
			callable(FormatSTD::Reader::readGraph(filename));
		case SIMPLE:
			callable(FormatSimple::Reader::readGraph(filename));
		case FMI:
			callable(FormatFMI::Reader::readGraph(filename));
		case FMI_CH:
			break;
		}
		std::cerr << "Unknown input fileformat: " << format << std::endl;
		std::exit(1);
	}

	/* try to read with types suitable to be written with Writer; strip CHNode<>, but apply CHEdge<> */
	template<typename Writer>
	inline GraphInData<typename MakeCHNode<typename Writer::node_type>::base_node_type, MakeCHEdge<typename Writer::edge_type>> readGraphForWriter(FileFormat format, std::string const& filename)
	{
		return readGraph<typename MakeCHNode<typename Writer::node_type>::base_node_type, MakeCHEdge<typename Writer::edge_type>>(format, filename);
	}

	/* run callable with types suitable to be written with Writer for write_format; strip CHNode<>, but apply CHEdge<> */
	template<typename Callable>
	inline void readGraphForWriteFormat(FileFormat write_format, FileFormat read_format, std::string const& filename, Callable&& callable)
	{
		switch (write_format) {
		case STD:
			callable(readGraphForWriter<FormatSTD::Writer>(read_format, filename));
			return;
		case SIMPLE:
			callable(readGraphForWriter<FormatSimple::Writer>(read_format, filename));
			return;
		case FMI:
			break;
		case FMI_CH:
			callable(readGraphForWriter<FormatFMI_CH::Writer>(read_format, filename));
			return;
		}
		std::cerr << "Unknown output fileformat: " << write_format << std::endl;
		std::exit(1);
	}

	template<typename Writer, typename NodeT, typename EdgeT>
	inline void writeCHGraphFile(std::string const& filename, GraphCHOutData<NodeT, EdgeT> const& data)
	{
		std::ofstream os(filename.c_str());
		if (!os.is_open()) {
			std::cerr << "FATAL_ERROR: Couldn't open graph file \'" <<
				filename << "\'. Exiting." << std::endl;
			std::abort();
		}

		Print("Exporting to " << filename);
		Writer::writeCHGraph(os, data);
		os.close();
	}

	template<typename NodeT, typename EdgeT>
	inline void writeCHGraphFile(FileFormat format, std::string const& filename, GraphCHOutData<NodeT, EdgeT> const& data)
	{
		switch (format) {
		case STD:
			writeCHGraphFile<FormatSTD::Writer>(filename, data);
			return;
		case SIMPLE:
			writeCHGraphFile<FormatSimple::Writer>(filename, data);
			return;
		case FMI:
			break;
		case FMI_CH:
			writeCHGraphFile<FormatFMI_CH::Writer>(filename, data);
			return;
		}
		std::cerr << "Unknown output fileformat: " << format << std::endl;
		std::exit(1);
	}
}


#endif
