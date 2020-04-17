#include <iostream>
#include <cassert>
#include "ProReader.h"
#include <map>
#include <cmath>
#include "../lib/argparse.hpp"
#include "utils.h"
#include "tests.h"

using namespace slicer;

argparse::ArgumentParser argumentParser;

std::vector<slicer::XYPoint> xyPoints;
std::vector<slicer::GeoPoint> geoPoints;

bool geoFlag = false;
bool testFlag = false;

int median_count = -1;
int mean_count = -1;

struct SlicePoint {
//	SlicePoint (GeoPoint aGeo, XYPoint aXY, double aBright, double aMean, double aMedian, double aDist, double aTemp)
//			:geo(aGeo),
//			 xy(aXY),
//			 brightness(aBright),
//			 mean(aMean),
//			 median(aMedian),
//			 distance(aDist),
//			 temperature(aTemp) {
//	}
	GeoPoint geo;
	XYPoint xy;
	double brightness, mean = -1, median = -1, distance, temperature;
};

bool parse_args (int argc, char* argv[]) {
	assert(argc != 1 && "ERROR: invalid argument count!");

	argumentParser = argparse::ArgumentParser("slicer");

	argumentParser.add_argument("--test")
			.help("Run simple tests")
			.default_value(false)
			.implicit_value(true);

	argumentParser.add_argument("-o", "--out")
			.help("Path to the output file without an extension")
			.required();

	argumentParser.add_argument("-i", "--in")
			.help("Path to the input file with an 'pro' extension")
			.required();

	argumentParser.add_argument("-g", "--geo")
			.help("Informs the application that coordinates will be provided in geographic format except of cartesian")
			.default_value(false)
			.implicit_value(true);

	argumentParser.add_argument("coordinates")
			.help("Coordinates of start and finish point in cartesian or geographic reference system")
			.required()
			.nargs(4)
			.action([] (const std::string& value) { return std::stof(value); });

	argumentParser.add_argument("--mean")
			.help("Set number of points for a perpendicular for mean calculation")
			.default_value(false)
			.implicit_value(true)
			.action([] (const std::string& value) { return std::stoi(value); });

	argumentParser.add_argument("--median")
			.help("Set number of points for a perpendicular for median calculation")
			.default_value(false)
			.implicit_value(true)
			.action([] (const std::string& value) { return std::stoi(value); });

	try {
		argumentParser.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << argumentParser;
		exit(0);
	}

	testFlag = argumentParser["--test"] == true;
	if (testFlag) {
		return true;
	}

	geoFlag = argumentParser["--geo"] == true;
	auto points = argumentParser.get<std::vector<float>>("coordinates");
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

	mean_count = argumentParser.get<int>("--mean");
	median_count = argumentParser.get<int>("--median");

	return true;
}

std::vector<double> getPerpendicularBrightness (const slicer::ProReader& reader, const int n, slicer::GeoPoint aPoint,
		double dlon, double dlat) {
	std::vector<double> result;

	result.push_back(reader.getBrightness(reader.geoToXY(aPoint)));

	for (auto i = 1; i < n + 1; ++i) {
		auto p1 = aPoint, p2 = aPoint;
		p1.lon += i * dlon;
		p1.lat -= i * dlat;
		auto xy_point = reader.geoToXY(p1);
		if (reader.isXYinRange(xy_point)) {
			result.push_back(reader.getBrightness(xy_point));
		}
		p2.lon -= i * dlon;
		p2.lat += i * dlat;
		xy_point = reader.geoToXY(p2);
		if (reader.isXYinRange(xy_point)) {
			result.push_back(reader.getBrightness(xy_point));
		}
	}

	return result;
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

	std::vector<SlicePoint> result;

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

	const auto firstGeo = reader.xyToGeo(intermediate_points[0]),
			lastGeo = reader.xyToGeo(intermediate_points[intermediate_points.size()]);
	for (auto& point : intermediate_points) {
		SlicePoint data;
		data.xy = point;
		data.geo = reader.xyToGeo(point);
		data.brightness = reader.getBrightness(point);
		data.temperature = reader.getCelsius(data.brightness);
		data.distance = getGeoDistance(data.geo, firstGeo);
		if (argumentParser["--mean"] == true) {
			auto dlon = (lastGeo.lon - firstGeo.lon) / mean_count,
					dlat = (lastGeo.lat - firstGeo.lat) / mean_count;
			data.mean = findMean(getPerpendicularBrightness(reader, mean_count, data.geo, dlon, dlat));
		}

		if (argumentParser["--median"] == true) {
			auto dlon = (lastGeo.lon - firstGeo.lon) / mean_count,
					dlat = (lastGeo.lat - firstGeo.lat) / mean_count;
			data.mean = findMedian(getPerpendicularBrightness(reader, median_count, data.geo, dlon, dlat));
		}

		if (geoFlag) {
			auto tmp = reader.xyToGeo(point);
			//std::cout << "lon: " << tmp.lon << ", lat: " << tmp.lat << std::endl;
		} else {
			//std::cout << "x: " << point.x << ", y: " << point.y << std::endl;
		}
//		std::cout << reader.getBrightness(point) << std::endl;
	}

	return 0;
}
