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
		XYPoint () = default;
		XYPoint (int aX, int aY)
				:x{ aX }, y{ aY } {
		}
		int x, y;
	};

	struct GeoPoint {
		GeoPoint () = default;
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

	static int getDistance (const XYPoint& aStart, const XYPoint& aFinish) {
		return std::max(std::abs(aStart.x - aFinish.x), std::abs(aStart.y - aFinish.y));
	}

	static double getGeoDistance (GeoPoint aStart, GeoPoint aFinish) {
		aStart.toRadian();
		aFinish.toRadian();
		auto sin1 = std::sin((aStart.lat - aFinish.lat) / 2);
		auto sin2 = std::sin((aStart.lon - aFinish.lon) / 2);
		auto a = sin1 * sin1 + sin2 * sin2 * std::cos(aStart.lat) * std::cos(aFinish.lat);
		auto c = 2 * std::atan2(std::sqrt(a), sqrt(1 - a));
		return c * EarthRadius;
	}

	static int lerp (int start, int finish, const float t) {
		return static_cast<int>(std::round(start + t * (finish - start)));
	}

	static XYPoint lerpPoint (const XYPoint& aStart, const XYPoint& aFinish, const float t) {
		return { lerp(aStart.x, aFinish.x, t), lerp(aStart.y, aFinish.y, t) };

	}

	static std::vector<XYPoint> getPoints (const XYPoint& aStart, const XYPoint& aFinish) {
		std::vector<XYPoint> res;
		auto n = getDistance(aStart, aFinish);
		for (auto i = 0; i <= n; ++i) {
			auto t = n == 0 ? 0.f : 1.0f * i / n;
			res.push_back(lerpPoint(aStart, aFinish, t));
		}
		return res;
	}

	static double findMean (std::vector<double> a) {
		double sum = 0;
		for (auto el : a) {
			sum += el;
		}
		return sum / a.size();
	}

	static double findMedian (std::vector<double> a) {
		std::sort(a.begin(), a.end());
		if (a.size() % 2 != 0) {
			return a[a.size() / 2];
		} else {
			return (a[(a.size() - 1) / 2] + a[a.size() / 2]) / 2.0;
		}
	}
} // slicer namespace
