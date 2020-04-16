//
// Created by TrueFinch on 17.04.2020.
//
#pragma once

#include <vector>
#include <iostream>
#include "ProReader.h"

namespace tests {

	struct Test {
		slicer::PointXY xy_data;
		slicer::PointGeo geo_expected;
	};

	void run_tests (slicer::ProReader& reader) {
		auto tests = std::vector<Test>{
				{{ 432, 585 }, { 142.6168, 39.3727 }},
				{{ 149, 149 }, { 138.94, 43.61 }}
		};
		auto i = 1;
		for (auto& test : tests) {
			auto result = reader.xyToGeo(test.xy_data);
			if (fabsf(result.lon - test.geo_expected.lon) < 1 && fabsf(result.lat - test.geo_expected.lat) < 1) {
				std::cout << "TEST " << i++ << " passed" << std::endl;
				std::cout << "expected: " << test.geo_expected.lon << ", " << test.geo_expected.lat << std::endl;
				std::cout << "got: " << result.lon << ", " << result.lat << std::endl;
			}
			else {
				std::cout << "TEST " << i++ << " failed" << std::endl;
				std::cout << "expected: " << test.geo_expected.lon << ", " << test.geo_expected.lat << std::endl;
				std::cout << "got: " << result.lon << ", " << result.lat << std::endl;
			}
		}
	}
}

