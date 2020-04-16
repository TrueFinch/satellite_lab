#include <iostream>
#include <cassert>
#include "ProReader.h"
#include <map>
#include <cmath>
#include "../lib/argparse.hpp"
#include "utils.h"
#include "tests.h"

slicer::PointGeo geo{ -1, -1 };
slicer::PointXY xy{ -1, -1 };

std::vector<slicer::PointGeo> geoPoints;
std::vector<slicer::PointXY> xyPoints;

bool geoFlag = false;
bool xyFlag = false;
bool testFlag = false;
bool graphFlag = false;

void parse_args (int argc, char* argv[]) {
	assert(argc != 1 && "ERROR: invalid argument count");

	argparse::ArgumentParser argument_parser("slicer");

	argument_parser.add_argument("-o", "--out")
			.help("Path to the output file without an extension")
			.required();

	argument_parser.add_argument("-i", "--in")
			.help("Path to the input file with an 'pro' extension")
			.required();

	argument_parser.add_argument("-g", "--geo")
			.help("Informs the application that coordinates will be provided in geographic format except of cartesian")
			.default_value(false)
			.implicit_value(true);

	argument_parser.add_argument("coordinates")
			.help("Coordinates of start and finish point in cartesian or geographic reference system")
			.required()
			.nargs(4);

	try {
		argument_parser.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << argument_parser;
		exit(0);
	}
}

// -geo lon lat -xy x y -test
int main (int argc, char* argv[]) {
	parse_args(argc, argv);

	slicer::ProReader reader;
	std::string path = std::string(argv[1]);
	reader.read(path);

	if (graphFlag) {
		auto points = getPoints(xyPoints[0], xyPoints[1]);
		for (auto& point : points) {
			std::cout << /*point.x << "," << point.y << "," <<*/ reader.getBrightness(point) << std::endl;
		}
	}
	else if (testFlag) {
		tests::run_tests(reader);
	}
	else if (geoFlag) {
		auto xy_point = reader.geoToXY(geo);
		std::cout << "x: " << xy_point.x << ", y: " << xy_point.y << std::endl;
		std::cout << "Brightness: " << reader.getBrightness(xy_point);
	}
	else if (xyFlag) {
		auto geo_point = reader.xyToGeo(xy);
		std::cout << "lon: " << geo_point.lon << ", lat: " << geo_point.lat << std::endl;
		std::cout << "Brightness: " << reader.getBrightness(xy);
	}

	return 0;
}
