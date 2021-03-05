/*
 * EstimatorOptions.h
 *
 *  Created on: Jun 16, 2013
 *      Author: steve
 */

#ifndef ESTIMATOROPTIONS_H_
#define ESTIMATOROPTIONS_H_

#include "fit/data/DataStructs.h"

#include <utility>

class EstimatorOptions {
  bool with_integral_scaling;

  DataStructs::DimensionRange fit_range_x;
  DataStructs::DimensionRange fit_range_y;

public:
  EstimatorOptions();
  virtual ~EstimatorOptions();
  bool isWithIntegralScaling() const;
  void setWithIntegralScaling(bool with_integral_scaling_);

  void setFitRangeX(DataStructs::DimensionRange &fit_range_);
  void setFitRangeY(DataStructs::DimensionRange &fit_range_);

  const DataStructs::DimensionRange &getFitRangeX() const;
  const DataStructs::DimensionRange &getFitRangeY() const;

  bool operator<(const EstimatorOptions &rhs) const;
  bool operator>(const EstimatorOptions &rhs) const;
  bool operator==(const EstimatorOptions &fit_options) const;
  bool operator!=(const EstimatorOptions &fit_options) const;

  friend std::ostream &operator<<(std::ostream &os,
                                  const EstimatorOptions &est_options);
};

#endif /* ESTIMATOROPTIONS_H_ */
