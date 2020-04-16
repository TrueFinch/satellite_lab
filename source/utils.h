//
// Created by TrueFinch on 13.03.2020.
//

#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>
#include <map>
#include "ProReader.h"
#include "../lib/argparse.hpp"

namespace slicer {

	struct PointXY {
		int x, y;
	};
	struct PointGeo {
		float lon, lat;
	};

	static float getDistance (const PointXY& aStart, const PointXY& aFinish) {
		return std::max(std::fabs(aStart.x - aFinish.x), std::fabs(aStart.y - aFinish.y));
	}

	static int lerp (int start, int finish, const float t) {
		return static_cast<int>(std::round(start + t * (finish - start)));
	}

	static PointXY lerpPoint (const PointXY& aStart, const PointXY& aFinish, const float t) {
		return { lerp(aStart.x, aFinish.x, t), lerp(aStart.y, aFinish.y, t) };

	}

	static std::vector<PointXY> getPoints (const PointXY& aStart, const PointXY& aFinish) {
		std::vector<PointXY> res;
		auto n = getDistance(aStart, aFinish);
		for (auto i = 0; i <= n; ++i) {
			auto t = n == 0 ? 0.f : i / n;
			res.push_back(lerpPoint(aStart, aFinish, t));
		}
		return res;
	}
} // utils namespace
