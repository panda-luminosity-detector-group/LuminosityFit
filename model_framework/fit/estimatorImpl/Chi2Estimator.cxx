/*
 * Chi2Estimator.cxx
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#include "Chi2Estimator.h"
#include "core/Model.h"
#include "fit/data/Data.h"

#include <iostream>

Chi2Estimator::Chi2Estimator() : ModelEstimator(false) {
  // TODO Auto-generated constructor stub
}

Chi2Estimator::~Chi2Estimator() {
  // TODO Auto-generated destructor stub
}

mydouble Chi2Estimator::eval(std::shared_ptr<Data> data) {
  // calculate chisquare
  mydouble chisq = 0.0;
  mydouble delta;

  std::vector<DataPointProxy> &data_points = data->getData();
  // loop over data
  for (unsigned int i = 0; i < data_points.size(); i++) {
    std::shared_ptr<DataStructs::binned_data_point> data_point;
    if (data_points[i].isPointUsed()) {
      data_point = data_points[i].getBinnedDataPoint();
      delta =
          (data_point->z - fit_model->evaluate(data_point->bin_center_value) *
                               data->getBinningFactor());
      mydouble weightsquare(1.0);
      if (data_point->z_error != 0.0)
        weightsquare = data_point->z_error * data_point->z_error;
      mydouble modelweight = 0.0;
      /*	if (delta > 0.0) {
                      modelweight += data->getBinningFactor()
                                      * fit_model->getUncertaincy(
                                                      data_point->bin_center_value).second;
         // take upper error of model (second) } else { modelweight +=
         data->getBinningFactor()
                                      * fit_model->getUncertaincy(
                                                      data_point->bin_center_value).first;
         // take lower error of model (first)
              }
              weightsquare += modelweight * modelweight;*/
      chisq += delta * delta / weightsquare;
    }
  }
  return chisq;
}
