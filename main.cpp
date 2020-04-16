#include <iostream>
#include <cassert>
#include "ProReader.h"
#include <map>
#include <cmath>

utils::PointGeo geo{-1, -1};
utils::PointXY xy{-1, -1};

std::vector<utils::PointGeo> geoPoints;
std::vector<utils::PointXY> xyPoints;

bool geoFlag = false;
bool xyFlag = false;
bool testFlag = false;
bool graphFlag = false;

void parse_args(int argc, char *argv[]) {
  for (auto i = 2; i < argc;) {
    auto foo = std::string(argv[i]);
    if (foo == "-graph") {
      graphFlag = true;
      i += 1;
    } else if (foo == "-geo") {
      geoFlag = true;
      geoPoints.push_back({std::stof(std::string(argv[i + 1])), std::stof(std::string(argv[i + 2]))});
      i += 2;
    } else if (foo == "-xy") {
      xyFlag = true;
      xyPoints.push_back({std::stoi(std::string(argv[i + 1])), std::stoi(std::string(argv[i + 2]))});
      i += 2;
    } else if (foo == "-test") {
      testFlag = true;
      break;
    } else {
      ++i;
    }
  }
}

struct Test {
  utils::PointXY xy_data;
  utils::PointGeo geo_expected;
};

void run_tests(lab1::ProReader &reader) {
  auto tests = std::vector<Test>{
      {{432, 585}, {142.6168, 39.3727}},
      {{149, 149}, {138.94, 43.61}}
  };
  auto i = 1;
  for (auto &test : tests) {
    auto result = reader.xyToGeo(test.xy_data);
    if (fabsf(result.lon - test.geo_expected.lon) < 1 && fabsf(result.lat - test.geo_expected.lat) < 1) {
      std::cout << "TEST " << i++ << " passed" << std::endl;
      std::cout << "expected: " << test.geo_expected.lon << ", " << test.geo_expected.lat << std::endl;
      std::cout << "got: " << result.lon << ", " << result.lat << std::endl;
    } else {
      std::cout << "TEST " << i++ << " failed" << std::endl;
      std::cout << "expected: " << test.geo_expected.lon << ", " << test.geo_expected.lat << std::endl;
      std::cout << "got: " << result.lon << ", " << result.lat << std::endl;
    }
  }
}

// -geo lon lat -xy x y -test
int main(int argc, char *argv[]) {
  assert(argc != 1 && "ERROR: invalid argument count");
  lab1::ProReader reader;
  std::string path = std::string(argv[1]);
  reader.read(path);

  parse_args(argc, argv);

  if (graphFlag) {
    auto points = utils::getPoints(xyPoints[0], xyPoints[1]);
    for (auto &point : points) {
      std::cout << /*point.x << "," << point.y << "," <<*/ reader.getBrightness(point) << std::endl;
    }
  } else if (testFlag) {
    run_tests(reader);
  } else if (geoFlag) {
    auto xy_point = reader.geoToXY(geo);
    std::cout << "x: " << xy_point.x << ", y: " << xy_point.y << std::endl;
    std::cout << "Brightness: " << reader.getBrightness(xy_point);
  } else if (xyFlag) {
    auto geo_point = reader.xyToGeo(xy);
    std::cout << "lon: " << geo_point.lon << ", lat: " << geo_point.lat << std::endl;
    std::cout << "Brightness: " << reader.getBrightness(xy);
  }

  return 0;
}
