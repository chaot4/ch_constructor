#include "parser.h"

namespace chc
{

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
		std::cerr << "Unknown fileformat!" << std::endl;
	}

	return FMI;
}

namespace Parser {
	/* Nodes */
	OSMNode readOSMNode(std::stringstream& ss)
	{
		OSMNode node;
		ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;
		return node;
	}

	bool writeOSMNode(OSMNode const& node, std::ofstream& os)
	{
		os << node.id << " " << node.osm_id << " " << node.lat << " " <<
			node.lon << " " << node.elev << std::endl;
		return true;
	}

	GeoNode readGeoNode(std::stringstream& ss)
	{
		GeoNode node;
		ss >> node.lat >> node.lon >> node.elev;
		return node;
	}

	bool writeGeoNode(GeoNode const& node, std::ofstream& os)
	{
		os << node.lat << " " << node.lon << " " << node.elev << std::endl;
		return true;
	}

	/* Edges */
	OSMEdge readOSMEdge(std::stringstream& ss)
	{
		OSMEdge edge;
		ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;
		return edge;
	}

	bool writeOSMEdge(OSMEdge const& edge, std::ofstream& os)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << " " <<
			edge.type << " " << edge.speed << std::endl;
		return true;
	}

	Edge readEdge(std::stringstream& ss)
	{
		Edge edge;
		ss >> edge.src >> edge.tgt >> edge.dist;
		return edge;
	}

	bool writeEdge(Edge const& edge, std::ofstream& os)
	{
		os << edge.src << " " << edge.tgt << " " << edge.dist << std::endl;
		return true;
	}

}

}
