#include "UnbinnedLogLikelihoodEstimator.h"
#include "core/Model.h"
#include "fit/data/Data.h"

#include <cmath>
#include <iostream>

UnbinnedLogLikelihoodEstimator::UnbinnedLogLikelihoodEstimator()
    : ModelEstimator(true), last_integral(0.0), skip_integral(false),
      integral_skip_counter(0), integral_skip_amount(5) {
  // TODO Auto-generated constructor stub
}

UnbinnedLogLikelihoodEstimator::~UnbinnedLogLikelihoodEstimator() {
  // TODO Auto-generated destructor stub
}

mydouble UnbinnedLogLikelihoodEstimator::eval(std::shared_ptr<Data> data) {
  // calculate loglikelihood
  // poisson sum_i(y_i * ln (f(x_i)) - f(x_i))

  mydouble loglikelihood = 0.0;

  std::vector<DataPointProxy> &data_points = data->getData();
  // loop over data
  for (unsigned int i = 0; i < data_points.size(); i++) {
    std::shared_ptr<DataStructs::unbinned_data_point> data_point;
    if (data_points[i].isPointUsed()) {
      data_point = data_points[i].getUnbinnedDataPoint();
      mydouble model_value = fit_model->evaluate(data_point->x);
      // if model is zero at this point should be removed otherwise log(0)!!!
      if (model_value <= 0.0)
        continue;

      loglikelihood += log(model_value);
    }
  }

  // normalization integral
  std::cout << "calculating normalization integral..." << std::endl;
  std::vector<DataStructs::DimensionRange> int_ranges;
  int_ranges.push_back(estimator_options.getFitRangeX());
  int_ranges.push_back(estimator_options.getFitRangeY());

  mydouble integral = fit_model->Integral(int_ranges, 1e-3);
  std::cout << "calculated!" << std::endl;
  std::cout << data_points.size() << std::endl;
  /*if (!skip_integral) {
   if (integral > 0.0) {
   if (fabs(integral - last_integral) / integral < 1e-2) {
   skip_integral = false;
   integral_skip_counter = integral_skip_amount;
   }
   }
   last_integral = integral;
   std::cout << "calculated!" << std::endl;
   } else {
   if (integral_skip_counter < 0)
   skip_integral = false;
   else
   --integral_skip_counter;
   }*/

  return -(loglikelihood - data_points.size() * integral);
}
