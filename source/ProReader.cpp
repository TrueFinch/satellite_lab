//
// Created by TrueFinch on 12.03.2020.
//

#include <cassert>
#include "ProReader.h"
#include <cmath>

using namespace slicer;

void ProReader::read (const std::string& aFilePath) {
	m_input.open(aFilePath, std::ios::in | std::ios::binary);
	assert(m_input.is_open() && "ERROR: can not open file!");
	if (!m_input.is_open()) {
		return;
	}
	readPassport();
	readImageData();
}

void ProReader::readPassport () {
	m_input.read(reinterpret_cast<char*>(&m_pass.FFh1), sizeof(m_pass.FFh1));
	m_input.read(m_pass.IS3name.data(), 13);
	m_input.read(reinterpret_cast<char*>(&m_pass.IS3id), sizeof(m_pass.IS3id));
	m_input.read(reinterpret_cast<char*>(&m_pass.coilNumber), sizeof(m_pass.coilNumber));
	m_input.read(reinterpret_cast<char*>(&m_pass.startDate), sizeof(m_pass.startDate));
	m_input.read(reinterpret_cast<char*>(&m_pass.dayOfYear), sizeof(m_pass.dayOfYear));
	m_input.read(reinterpret_cast<char*>(&m_pass.startMilis), sizeof(m_pass.startMilis));

	// skip reserved and service bytes
	m_input.ignore(42);

	{
		uint16_t buf = 0;
		m_input.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		m_pass.projType = buf == 1 ? eProjType::MERCATOR :
						  buf == 2 ? eProjType::EQUIDISTANT : eProjType::NONE;
		assert(m_pass.projType != eProjType::NONE && "ERROR: unknown projection type!");
	}

	m_input.read(reinterpret_cast<char*>(&m_pass.linesCount), sizeof(m_pass.linesCount));
	m_input.read(reinterpret_cast<char*>(&m_pass.pixelsCount), sizeof(m_pass.pixelsCount));

	static_assert(sizeof(float) == 4, "ERROR: float size are not enough");
	m_input.read(reinterpret_cast<char*>(&m_pass.latitude), sizeof(m_pass.latitude));
	m_input.read(reinterpret_cast<char*>(&m_pass.longitude), sizeof(m_pass.longitude));
	m_input.read(reinterpret_cast<char*>(&m_pass.latitudeSize), sizeof(m_pass.latitudeSize));
	m_input.read(reinterpret_cast<char*>(&m_pass.longitudeSize), sizeof(m_pass.longitudeSize));
	m_input.read(reinterpret_cast<char*>(&m_pass.latitudeStep), sizeof(m_pass.latitudeStep));
	m_input.read(reinterpret_cast<char*>(&m_pass.longitudeStep), sizeof(m_pass.longitudeStep));
	m_input.read(reinterpret_cast<char*>(&m_pass.coefficientA), sizeof(m_pass.coefficientA));
	m_input.read(reinterpret_cast<char*>(&m_pass.coefficientB), sizeof(m_pass.coefficientB));

	m_input.ignore(394);
}

void ProReader::readImageData () {
	m_input.seekg(512, std::ios::beg);
	auto size = m_pass.linesCount * m_pass.pixelsCount;

	pixelsBrightness.resize(size);
	uint16_t buf;
	for (auto i = 0; i < size; ++i) {
		m_input.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		pixelsBrightness[i] = buf;
	}
}

GeoPoint ProReader::xyToGeo (const XYPoint& aPoint) const {
	auto y = getLinesCount() - aPoint.y - 1;
	auto res_lon = getLongitude()
			+ static_cast<float>(aPoint.x) * getLongitudeSize() / static_cast<float>(getPixelsCount() - 1);
	auto width = static_cast<float>(getPixelsCount()) / getLongitudeSize() * 360.0f / (2 * M_PI);
	auto map_offset = width * std::log(std::tan(M_PI / 4) + (toRadian(getLatitude()) / 2));
	auto a = (getLinesCount() + map_offset - aPoint.y - 0.52) / width;
	auto res_lat = static_cast<float>(180 / M_PI * (2 * std::atan(std::exp(a)) - M_PI / 2));
	return { res_lon, res_lat };
}

XYPoint ProReader::geoToXY (const GeoPoint& aPoint) const {
	auto col = (aPoint.lon - getLongitude()) * (static_cast<float>(getPixelsCount() - 1) / getLongitudeSize());
	auto res_x = static_cast<int>(std::floor(col + 0.5));
	auto width = static_cast<float>(getPixelsCount()) / getLongitudeSize() * 360.0f / (2 * M_PI);
	auto map_offset = width * std::log(std::tan(M_PI / 4) + (toRadian(getLatitude()) / 2));
	auto log = std::log(std::tan(M_PI / 4) + toRadian(aPoint.lat) / 2);
	auto estimated = getLinesCount() - (width * log - map_offset) - 0.5;
	auto res_y = static_cast<int>(std::floor(estimated + 0.5));
	return { res_x, res_y };
}

float ProReader::toMercatorLat (const float aLat) {
	auto lat = aLat * std::atan2(1.f, 1.f) / 90.f;
	lat = std::log(std::tan(0.5f * lat + std::atan2(1.f, 1.f)));
	lat = 90.f * lat / std::atan2(1.f, 1.f);
	return lat;
}

float ProReader::toUnmercatorLat (const float aLat) {
	auto lat = aLat * std::atan2(1.f, 1.f) / 90.f;
	lat = 2.f * (std::atan(std::exp(lat)) - std::atan2(1.f, 1.f));
	lat = 90.f * lat / std::atan2(1, 1);
	return lat;
}

float ProReader::toRadian (float aAngle) {
	return aAngle * M_PI / 180;
}

float ProReader::toDegree (float aAngle) {
	return aAngle * 180 / M_PI;
}

uint16_t ProReader::getBrightness (const XYPoint& aPoint) const {
	return pixelsBrightness[aPoint.x + getPixelsCount() * (getLinesCount() - aPoint.y - 1)];
}

float ProReader::getCelsius (const int brightness) const {
	return m_pass.coefficientA * brightness + m_pass.coefficientB;
}

