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

Parser::STDNode::STDNode()
	: id(c::NO_NID), osm_id(0), lat(0), lon(0), elev(0) {}
Parser::STDNode::STDNode(NodeID id, uint osm_id, double lat, double lon, int elev)
	: id(id), osm_id(osm_id), lat(lat), lon(lon), elev(elev) {}

Parser::SIMPLENode::SIMPLENode()
	: lat(0), lon(0), elev(0) {}
Parser::SIMPLENode::SIMPLENode(double lat, double lon, int elev)
	: lat(lat), lon(lon), elev(elev) {}

Parser::STDEdge::STDEdge()
	: src(c::NO_NID), tgt(c::NO_NID), dist(0), type(0), speed(-1) {}
Parser::STDEdge::STDEdge(NodeID src, NodeID tgt, uint dist, uint type, int speed)
	: src(src), tgt(tgt), dist(dist), type(type), speed(speed) {}

Parser::SIMPLEEdge::SIMPLEEdge()
	: src(c::NO_NID), tgt(c::NO_NID), dist(0) {}
Parser::SIMPLEEdge::SIMPLEEdge(NodeID src, NodeID tgt, uint dist)
	: src(src), tgt(tgt), dist(dist) {}

/* 
 * Node casts
 */

void Parser::cast(STDNode const& in_node, Node& out_node)
{
	out_node = Node(in_node.id);
}

void Parser::cast(SIMPLENode const& in_node, Node& out_node)
{
	out_node = Node(c::NO_NID);
}

void Parser::cast(Node const& in_node, STDNode& out_node)
{
	out_node = STDNode(in_node.id, 0, 0, 0, 0);
}

void Parser::cast(Node const& in_node, SIMPLENode& out_node)
{
	out_node = SIMPLENode(0, 0, 0);
}

/* 
 * Edge casts
 */

void Parser::cast(STDEdge const& in_edge, Edge& out_edge)
{
	out_edge = Edge(c::NO_EID, in_edge.src, in_edge.tgt, in_edge.dist);
}

void Parser::cast(SIMPLEEdge const& in_edge, Edge& out_edge)
{
	out_edge = Edge(c::NO_EID, in_edge.src, in_edge.tgt, in_edge.dist);
}

void Parser::cast(Edge const& in_edge, STDEdge& out_edge)
{
	out_edge = STDEdge(in_edge.src, in_edge.tgt, in_edge.dist, 0, -1);
}

void Parser::cast(Edge const& in_edge, SIMPLEEdge& out_edge)
{
	out_edge = SIMPLEEdge(in_edge.src, in_edge.tgt, in_edge.dist);
}

/* Nodes */
Parser::STDNode Parser::readSTDNode(std::stringstream& ss)
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

Parser::SIMPLENode Parser::readSIMPLENode(std::stringstream& ss)
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
Parser::STDEdge Parser::readSTDEdge(std::stringstream& ss)
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

Parser::SIMPLEEdge Parser::readSIMPLEEdge(std::stringstream& ss)
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
