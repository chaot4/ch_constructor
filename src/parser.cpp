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

/* Nodes */
STDNode Parser::readSTDNode(std::stringstream& ss)
{
	STDNode node;
	ss >> node.id >> node.osm_id >> node.lat >> node.lon >> node.elev;
	return node;
}

bool Parser::writeSTDNode(STDNode const& node, std::ofstream& os)
{
	os << node.id << " " << node.osm_id << " " << node.lat << " " <<
		node.lon << " " << node.elev << std::endl;
	return true;
}

SIMPLENode Parser::readSIMPLENode(std::stringstream& ss)
{
	SIMPLENode node;
	ss >> node.lat >> node.lon >> node.elev;
	return node;
}

bool Parser::writeSIMPLENode(SIMPLENode const& node, std::ofstream& os)
{
	os << node.lat << " " << node.lon << " " << node.elev << std::endl;
	return true;
}

/* Edges */
STDEdge Parser::readSTDEdge(std::stringstream& ss)
{
	STDEdge edge;
	ss >> edge.src >> edge.tgt >> edge.dist >> edge.type >> edge.speed;
	return edge;
}

bool Parser::writeSTDEdge(STDEdge const& edge, std::ofstream& os)
{
	os << edge.src << " " << edge.tgt << " " << edge.dist << " " <<
		edge.type << " " << edge.speed << std::endl;
	return true;
}

SIMPLEEdge Parser::readSIMPLEEdge(std::stringstream& ss)
{
	SIMPLEEdge edge;
	ss >> edge.src >> edge.tgt >> edge.dist;
	return edge;
}

bool Parser::writeSIMPLEEdge(SIMPLEEdge const& edge, std::ofstream& os)
{
	os << edge.src << " " << edge.tgt << " " << edge.dist << std::endl;
	return true;
}

}
