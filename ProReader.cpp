//
// Created by TrueFinch on 12.03.2020.
//

#include <cassert>
#include "ProReader.h"
#include <math.h>

using namespace lab1;

void ProReader::read(const std::string &aFilePath) {
  m_input.open(aFilePath, std::ios::in | std::ios::binary);
  assert(m_input.is_open() && "ERROR: can not open file!");
  if (!m_input.is_open()) {
    return;
  }
  readPassport();
  readImageData();
}

void ProReader::readPassport() {
  m_input.read(reinterpret_cast<char *>(&m_pass.FFh1), sizeof(m_pass.FFh1));
  m_input.read(m_pass.IS3name.data(), 13);
  m_input.read(reinterpret_cast<char *>(&m_pass.IS3id), sizeof(m_pass.IS3id));
  m_input.read(reinterpret_cast<char *>(&m_pass.coilNumber), sizeof(m_pass.coilNumber));
  m_input.read(reinterpret_cast<char *>(&m_pass.startDate), sizeof(m_pass.startDate));
  m_input.read(reinterpret_cast<char *>(&m_pass.dayOfYear), sizeof(m_pass.dayOfYear));
  m_input.read(reinterpret_cast<char *>(&m_pass.startMilis), sizeof(m_pass.startMilis));

  // skip reserved and service bytes
  m_input.ignore(42);

  {
    uint16_t buf = 0;
    m_input.read(reinterpret_cast<char *>(&buf), sizeof(buf));
    m_pass.projType = buf == 1 ? eProjType::MERCATOR :
                      buf == 2 ? eProjType::EQUIDISTANT : eProjType::NONE;
    assert(m_pass.projType != eProjType::NONE && "ERROR: unknown projection type!");
  }

  m_input.read(reinterpret_cast<char *>(&m_pass.linesCount), sizeof(m_pass.linesCount));
  m_input.read(reinterpret_cast<char *>(&m_pass.pixelsCount), sizeof(m_pass.pixelsCount));

  static_assert(sizeof(float) == 4, "ERROR: float size are not enough");
  m_input.read(reinterpret_cast<char *>(&m_pass.latitude), sizeof(m_pass.latitude));
  m_input.read(reinterpret_cast<char *>(&m_pass.longitude), sizeof(m_pass.longitude));
  m_input.read(reinterpret_cast<char *>(&m_pass.latitudeSize), sizeof(m_pass.latitudeSize));
  m_input.read(reinterpret_cast<char *>(&m_pass.longitudeSize), sizeof(m_pass.longitudeSize));
  m_input.read(reinterpret_cast<char *>(&m_pass.latitudeStep), sizeof(m_pass.latitudeStep));
  m_input.read(reinterpret_cast<char *>(&m_pass.longitudeStep), sizeof(m_pass.longitudeStep));

  m_input.ignore(410);
}

void ProReader::readImageData() {
  m_input.seekg(512, std::ios::beg);
  auto size = m_pass.linesCount * m_pass.pixelsCount;

  pixelsBrightness.resize(size);
  uint16_t buf;
  for (auto i = 0; i < size; ++i) {
    m_input.read(reinterpret_cast<char *>(&buf), sizeof(buf));
    pixelsBrightness[i] = buf;
  }
}

utils::PointGeo ProReader::xyToGeo(const utils::PointXY &aPoint) const {
  auto y = getLinesCount() - aPoint.y - 1;
  auto res_lon =
      getLongitude() + static_cast<float>(aPoint.x) * getLongitudeSize() / static_cast<float>(getPixelsCount() - 1);
  float min_lat = -1, max_lat = -1;
  if (getProjType() == eProjType::MERCATOR) {
    min_lat = toMercatorLat(getLatitude());
    max_lat = toMercatorLat(getLatitude() + getLatitudeSize());
  } else {
    min_lat = getLatitude();
    max_lat = getLatitude() + getLatitudeSize();
  }
  auto res_lat = min_lat + y * (max_lat - min_lat) / static_cast<float>(getPixelsCount() - 1);
  res_lat = getProjType() == eProjType::MERCATOR ? res_lat : toUnmercatorLat(res_lat);
  return {res_lon, res_lat};
}

utils::PointXY ProReader::geoToXY(const utils::PointGeo &aPoint) const {
  auto col = (aPoint.lon - getLongitude()) / getLongitudeSize() * static_cast<float>(getPixelsCount() - 1);
  auto res_x = static_cast<int>(std::floor(col + 0.5));
  float min_lat = -1, max_lat = -1;
  if (getProjType() == eProjType::MERCATOR) {
    min_lat = toMercatorLat(getLatitude());
    max_lat = toMercatorLat(getLatitude() + getLatitudeSize());
  } else {
    min_lat = getLatitude();
    max_lat = getLatitude() + getLatitudeSize();
  }
  auto denominator = (max_lat - min_lat) / static_cast<float>(getLinesCount() - 1);
  auto line = ((getProjType() == eProjType::MERCATOR ? toMercatorLat(aPoint.lat) : aPoint.lat) - min_lat) / denominator;
  auto res_y = getLinesCount() - static_cast<int>(std::floor(line + 0.5f)) - 1;
  return {res_x, res_y};
}

float ProReader::toMercatorLat(const float aLat) {
  auto lat = aLat * std::atan2(1.f, 1.f) / 90.f;
  lat = std::log(std::tan(0.5f * lat + std::atan2(1.f, 1.f)));
  lat = 90.f * lat / std::atan2(1, 1);
  return lat;
}

float ProReader::toUnmercatorLat(const float aLat) {
  auto lat = aLat * std::atan2(1.f, 1.f) / 90.f;
  lat = 2.f * (std::atan(std::exp(lat)) - std::atan2(1.f, 1.f));
  lat = 90.f * lat / std::atan2(1, 1);
  return lat;
}

float ProReader::toRadian(float aAngle) {
  return aAngle * M_PI / 180;
}

float ProReader::toDegree(float aAngle) {
  return aAngle * 180 / M_PI;
}

uint16_t ProReader::getBrightness(const utils::PointXY &aPoint) const {
  return pixelsBrightness[aPoint.x + (getPixelsCount() * (getLinesCount() - aPoint.y - 1))];
}
