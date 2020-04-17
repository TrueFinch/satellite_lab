#include <iostream>
#include <cassert>
#include "ProReader.h"
#include <map>
#include <cmath>
#include "../lib/argparse.hpp"
#include "utils.h"
#include "tests.h"

std::vector<slicer::XYPoint> xyPoints;
std::vector<slicer::GeoPoint> geoPoints;

bool geoFlag = false;
bool testFlag = false;

bool parse_args (int argc, char* argv[]) {
	assert(argc != 1 && "ERROR: invalid argument count!");

	argparse::ArgumentParser argument_parser("slicer");

	argument_parser.add_argument("--test")
			.help("Run simple tests")
			.default_value(false)
			.implicit_value(true);

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
			.remaining()
			.action([] (const std::string& value) { return std::stof(value); });

	try {
		argument_parser.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << argument_parser;
		exit(0);
	}


	testFlag = argument_parser["--test"] == true;
	if (testFlag) {
		return true;
	}

	geoFlag = argument_parser["--geo"] == true;
	auto points = argument_parser.get<std::vector<float>>("coordinates");
	if (points.size() % 2 == 0) {
		assert(points.size() % 2 == 0 && "ERROR: invalid number of coordinates!");
		return false;
	}
	if (geoFlag) {
		for (auto i = 0; i < points.size(); i += 2) {
			geoPoints.emplace_back(points[i], points[i + 1]);
		}
	} else {
		for (auto i = 0; i < points.size(); i += 2) {
			xyPoints.emplace_back(static_cast<int>(points[i]), static_cast<int>(points[i + 1]));
		}
	}

	return true;
}

// -geo lon lat -xy x y -test
int main (int argc, char* argv[]) {
	if (!parse_args(argc, argv)) {
		return 0;
	}

	slicer::ProReader reader;
	std::string path = std::string(argv[1]);
	reader.read(path);

	if (testFlag) {
		tests::run_tests(reader);
		return 0;
	}

	std::vector<slicer::XYPoint> intermediate_points;

	if (geoFlag) {
		for (auto& point : geoPoints) {
			xyPoints.emplace_back(reader.geoToXY(point));
		}
		geoPoints.clear();
	}

	if (xyPoints.size() > 1) {
		intermediate_points = slicer::getPoints(intermediate_points[0], intermediate_points[1]);
	} else {
		intermediate_points = xyPoints;
	}

	for (auto& point : intermediate_points) {
		if (geoFlag) {
			auto tmp = reader.xyToGeo(point);
			std::cout << "lon: " << tmp.lon << ", lat: " << tmp.lat << std::endl;
		} else {
			std::cout << "x: " << point.x << ", y: " << point.y << std::endl;
		}
		std::cout << reader.getBrightness(point) << std::endl;
	}

	return 0;
}
