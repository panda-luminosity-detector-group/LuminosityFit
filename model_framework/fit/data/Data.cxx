/*
 * Data.cxx
 *
 *  Created on: Jun 15, 2013
 *      Author: steve
 */

#include "Data.h"

Data::Data(unsigned int dimension_)
    : data_points(), dimension(dimension_), binning_factor(1.0),
      is_binning_factor_set(false) {}

Data::~Data() {
  // TODO Auto-generated destructor stub
}

unsigned int Data::getDimension() const { return dimension; }

unsigned int Data::getNumberOfDataPoints() const { return data_points.size(); }

unsigned int Data::getNumberOfUsedDataPoints() const {
  unsigned int num_points = 0;

  for (unsigned int i = 0; i < data_points.size(); i++) {
    if (data_points[i].isPointUsed())
      num_points++;
  }
  return num_points;
}

double Data::getBinningFactor() const { return binning_factor; }

bool Data::isBinningFactorSet() const { return is_binning_factor_set; }

void Data::clearData() {
  data_points.clear();
  is_binning_factor_set = false;
}

void Data::insertData(std::vector<DataPointProxy> &data_points_) {
  for (unsigned int i = 0; i < data_points_.size(); i++) {
    insertData(data_points_[i]);
  }
}
void Data::insertData(DataPointProxy &data_point_) {
  if (data_point_.isBinnedDataPoint()) {
    if (!is_binning_factor_set) {
      is_binning_factor_set = true;
      binning_factor = 1.0;
      for (unsigned int i = 0; i < getDimension(); i++) {
        binning_factor *= data_point_.getBinnedDataPoint()->bin_widths[i];
      }
    }
  } else {
    is_binning_factor_set = false;
    binning_factor = 1.0;
  }
  data_points.push_back(data_point_);
}

std::vector<DataPointProxy> &Data::getData() { return data_points; }
