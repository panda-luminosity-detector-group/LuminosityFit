/*
 * Data.h
 *
 *  Created on: Jun 15, 2013
 *      Author: steve
 */

#ifndef DATA_H_
#define DATA_H_

#include "DataPointProxy.h"

#include <vector>

class Data {
private:
  // this is the vector that stores the information
  std::vector<DataPointProxy> data_points;

  // binning factor
  double binning_factor;
  bool is_binning_factor_set;

  // dimension of the data
  unsigned int dimension;

public:
  Data(unsigned int dimension_);
  virtual ~Data();

  unsigned int getDimension() const;
  unsigned int getNumberOfDataPoints() const;
  unsigned int getNumberOfUsedDataPoints() const;

  double getBinningFactor() const;
  bool isBinningFactorSet() const;

  void clearData();

  void insertData(std::vector<DataPointProxy> &data_points_);
  void insertData(DataPointProxy &data_point_);
  std::vector<DataPointProxy> &getData();
};

#endif /* BINNEDDATA_H_ */
