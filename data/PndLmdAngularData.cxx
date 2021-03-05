/*
 * PndLmdAngularData.h
 *
 *  Created on: Jun 27, 2012
 *      Author: steve
 */
#include "PndLmdAngularData.h"

ClassImp(PndLmdAngularData)

    PndLmdAngularData::PndLmdAngularData()
    : reference_luminosity_per_event(1.0), ip_offset_x(0.0), ip_offset_y(0.0) {}

PndLmdAngularData::PndLmdAngularData(const PndLmdAngularData &lmd_ang_data_)
    : PndLmdHistogramData(lmd_ang_data_) {
  reference_luminosity_per_event =
      lmd_ang_data_.getReferenceLuminosityPerEvent();
  ip_offset_x = lmd_ang_data_.ip_offset_x;
  ip_offset_y = lmd_ang_data_.ip_offset_y;
}

PndLmdAngularData::~PndLmdAngularData() {}

double PndLmdAngularData::getReferenceLuminosity() const {
  return reference_luminosity_per_event * getNumEvents();
}

double PndLmdAngularData::getReferenceLuminosityPerEvent() const {
  return reference_luminosity_per_event;
}

void PndLmdAngularData::setReferenceLuminosityPerEvent(
    double reference_luminosity_per_event_) {
  reference_luminosity_per_event = reference_luminosity_per_event_;
}

std::pair<double, double> PndLmdAngularData::getIPOffsets() const {
  return std::make_pair(ip_offset_x, ip_offset_y);
}

void PndLmdAngularData::setIPOffsets(
    const std::pair<double, double> &ip_offsets) {
  ip_offset_x = ip_offsets.first;
  ip_offset_y = ip_offsets.second;
}

bool PndLmdAngularData::operator<(const PndLmdAngularData &lmd_data) const {
  if (reference_luminosity_per_event <
      lmd_data.getReferenceLuminosityPerEvent()) {
    return true;
  } else if (reference_luminosity_per_event >
             lmd_data.getReferenceLuminosityPerEvent())
    return false;
  if (ip_offset_x < lmd_data.ip_offset_x) {
    return true;
  } else if (ip_offset_x > lmd_data.ip_offset_x)
    return false;
  if (ip_offset_y < lmd_data.ip_offset_y) {
    return true;
  } else if (ip_offset_y > lmd_data.ip_offset_y)
    return false;
  return PndLmdAbstractData::operator<(lmd_data);
}
bool PndLmdAngularData::operator>(const PndLmdAngularData &lmd_data) const {
  return (lmd_data < *this);
}
bool PndLmdAngularData::operator==(const PndLmdAngularData &lmd_data) const {
  if (reference_luminosity_per_event !=
      lmd_data.getReferenceLuminosityPerEvent())
    return false;
  if (ip_offset_x != lmd_data.ip_offset_x)
    return false;
  if (ip_offset_y != lmd_data.ip_offset_y)
    return false;
  return PndLmdAbstractData::operator==(lmd_data);
}
bool PndLmdAngularData::operator!=(const PndLmdAngularData &lmd_data) const {
  return !(*this == lmd_data);
}
