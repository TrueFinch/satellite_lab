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
	static double EarthRadius = 6371.21;
	struct XYPoint {
		XYPoint (int aX, int aY)
				:x{ aX }, y{ aY } {
		}
		int x, y;
	};

	struct GeoPoint {
		GeoPoint (float aLon, float aLat, bool aRadian = false)
				:lon{ aLon }, lat{ aLat }, isRadian{ aRadian } {
		}
		double lon, lat;
		bool isRadian;

		void toRadian () {
			if (isRadian) {
				return;
			}
			lon = degreeToRadian(lon);
			lat = degreeToRadian(lat);
			isRadian = !isRadian;
		}

		void toDegree () {
			if (!isRadian) {
				return;
			}
			lon = radianToDegree(lon);
			lat = radianToDegree(lat);
			isRadian = !isRadian;
		}
		static double degreeToRadian (double x) {
			return x * M_PI / 180;
		}

		static double radianToDegree (double x) {
			return x / M_PI * 180;
		}
	};

	int getDistance (const XYPoint& aStart, const XYPoint& aFinish) {
		return std::max(std::abs(aStart.x - aFinish.x), std::abs(aStart.y - aFinish.y));
	}

	double getDistance (GeoPoint aStart, GeoPoint aFinish) {
		aStart.toRadian();
		aFinish.toRadian();
		auto d = std::acos(
				std::sin(aStart.lat) * std::sin(aFinish.lat)
				+ std::cos(aStart.lat) * std::cos(aFinish.lat) * std::cos(aStart.lon - aFinish.lon)
				);
		return  d * EarthRadius;
	}

	int lerp (int start, int finish, const float t) {
		return static_cast<int>(std::round(start + t * (finish - start)));
	}

	XYPoint lerpPoint (const XYPoint& aStart, const XYPoint& aFinish, const float t) {
		return { lerp(aStart.x, aFinish.x, t), lerp(aStart.y, aFinish.y, t) };

	}

	std::vector<XYPoint> getPoints (const XYPoint& aStart, const XYPoint& aFinish) {
		std::vector<XYPoint> res;
		auto n = getDistance(aStart, aFinish);
		for (auto i = 0; i <= n; ++i) {
			auto t = n == 0 ? 0.f : 1.0f * i / n;
			res.push_back(lerpPoint(aStart, aFinish, t));
		}
		return res;
	}
} // slicer namespace
