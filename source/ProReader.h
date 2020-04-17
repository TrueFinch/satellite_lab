//
// Created by TrueFinch on 12.03.2020.
//
#pragma once

#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils.h"

namespace slicer {
	const size_t PassportSize = 512;

	enum class eProjType {
		NONE, MERCATOR, EQUIDISTANT
	};

	struct Passport {
		// const part of pass
		int8_t FFh1;
		std::array<char, 13> IS3name;
		uint32_t IS3id;
		uint32_t coilNumber;
		uint16_t startDate;
		uint16_t dayOfYear;
		uint32_t startMilis;

		// specific part of pass
		eProjType projType;
		uint16_t linesCount;
		uint16_t pixelsCount;
		float latitude; // широта
		float longitude; // долгота
		float latitudeSize;
		float longitudeSize;
		float latitudeStep;
		float longitudeStep;
		double coefficientA;
		double coefficientB;
	};

	class ProReader {
	public:
		void read (const std::string& aFilePath);
		GeoPoint xyToGeo (const XYPoint& aPoint) const;
		XYPoint geoToXY (const GeoPoint& aPoint) const;
		float getCelsius(const int brightness) const;
		uint16_t getBrightness (const XYPoint& aPoint) const;

		int8_t getFFh1 () const {
			return m_pass.FFh1;
		}
		std::array<char, 13> IS3name () const {
			return m_pass.IS3name;
		}
		uint32_t getIS3id () const {
			return m_pass.IS3id;
		}
		uint32_t getCoilNumber () const {
			return m_pass.coilNumber;
		}
		uint16_t getStartDate () const {
			return m_pass.startDate;
		}
		uint16_t getDayOfYear () const {
			return m_pass.dayOfYear;
		}
		uint32_t getStartMilis () const {
			return m_pass.startMilis;
		}
		eProjType getProjType () const {
			return m_pass.projType;
		}
		uint16_t getLinesCount () const {
			return m_pass.linesCount;
		}
		uint16_t getPixelsCount () const {
			return m_pass.pixelsCount;
		}
		float getLatitude () const {
			return m_pass.latitude;
		}
		float getLongitude () const {
			return m_pass.longitude;
		}
		float getLatitudeSize () const {
			return m_pass.latitudeSize;
		}
		float getLongitudeSize () const {
			return m_pass.longitudeSize;
		}
		float getLatitudeStep () const {
			return m_pass.latitudeStep;
		}
		float getLongitudeStep () const {
			return m_pass.longitudeStep;
		}

		static float toRadian (float aAngle);
		static float toDegree (float aAngle);
	private:
		void readPassport ();
		void readImageData ();
		/*
		 * Get degree, return degree
		 * */
		static float toMercatorLat (float aLat);
		/*
		 * Get degree, return degree
		 * */
		static float toUnmercatorLat (float aLat);

		Passport m_pass;
		std::ifstream m_input;
		std::vector<uint16_t> pixelsBrightness;
	};

}

