/*
 * EstimatorOptions.cxx
 *
 *  Created on: Jun 16, 2013
 *      Author: steve
 */

#include "EstimatorOptions.h"

#include <iostream>

EstimatorOptions::EstimatorOptions()
    : with_integral_scaling(true), fit_range_x(), fit_range_y() {}

EstimatorOptions::~EstimatorOptions() {
  
}

bool EstimatorOptions::isWithIntegralScaling() const {
  return with_integral_scaling;
}

void EstimatorOptions::setWithIntegralScaling(bool with_integral_scaling_) {
  with_integral_scaling = with_integral_scaling_;
}

const DataStructs::DimensionRange &EstimatorOptions::getFitRangeX() const {
  return fit_range_x;
}

const DataStructs::DimensionRange &EstimatorOptions::getFitRangeY() const {
  return fit_range_y;
}

void EstimatorOptions::setFitRangeX(DataStructs::DimensionRange &fit_range_) {
  fit_range_x = fit_range_;
}
void EstimatorOptions::setFitRangeY(DataStructs::DimensionRange &fit_range_) {
  fit_range_y = fit_range_;
}

bool EstimatorOptions::operator<(const EstimatorOptions &rhs) const {
  // check binary options first
  if (with_integral_scaling < rhs.isWithIntegralScaling())
    return true;
  else if (with_integral_scaling > rhs.isWithIntegralScaling())
    return false;

  if (fit_range_x < rhs.getFitRangeX())
    return true;
  else if (fit_range_x > rhs.getFitRangeX())
    return false;

  return (fit_range_y < rhs.getFitRangeY());
}

bool EstimatorOptions::operator>(const EstimatorOptions &rhs) const {
  return (rhs < *this);
}

bool EstimatorOptions::operator==(const EstimatorOptions &rhs) const {
  return ((*this < rhs) == (*this > rhs));
}

bool EstimatorOptions::operator!=(const EstimatorOptions &rhs) const {
  return !(*this == rhs);
}

std::ostream &operator<<(std::ostream &os,
                         const EstimatorOptions &est_options) {

  if (est_options.getFitRangeX().is_active) {
    os << "primary dimension lower fit range: "
       << est_options.getFitRangeX().range_low << std::endl;
    os << "primary dimension upper fit range: "
       << est_options.getFitRangeX().range_high << std::endl;
  }
  if (est_options.getFitRangeY().is_active) {
    os << "secondary dimension lower fit range: "
       << est_options.getFitRangeY().range_low << std::endl;
    os << "secondary dimension upper fit range: "
       << est_options.getFitRangeY().range_high << std::endl;
  }
  return os;
}
