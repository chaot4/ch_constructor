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

}
