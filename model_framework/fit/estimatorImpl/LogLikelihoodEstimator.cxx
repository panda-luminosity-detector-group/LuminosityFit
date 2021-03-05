#include "LogLikelihoodEstimator.h"
#include "core/Model.h"
#include "fit/data/Data.h"

#include <cmath>

LogLikelihoodEstimator::LogLikelihoodEstimator() : ModelEstimator(true) {
  // TODO Auto-generated constructor stub
}

LogLikelihoodEstimator::~LogLikelihoodEstimator() {
  // TODO Auto-generated destructor stub
}

mydouble LogLikelihoodEstimator::eval(std::shared_ptr<Data> data) {
  // calculate loglikelihood
  // poisson sum_i(y_i * ln (f(x_i)) - f(x_i))

  std::vector<DataPointProxy> &data_points = data->getData();

  // mydouble loglikelihood(0.0);
  // mydouble delta;

  std::vector<mydouble> deltas;
  deltas.reserve(data_points.size());

  // loop over data
  for (unsigned int i = 0; i < data_points.size(); i++) {
    std::shared_ptr<DataStructs::binned_data_point> data_point;
    if (data_points[i].isPointUsed()) {
      data_point = data_points[i].getBinnedDataPoint();
      mydouble model_value = fit_model->evaluate(data_point->bin_center_value) *
                             data->getBinningFactor();
      // if model is zero at this point should be removed otherwise log(0)!!!
      if (model_value <= 0.0)
        continue;

      // delta = model_value - data_point->z * log(model_value);
      // loglikelihood += delta;
      deltas.push_back(model_value);
      deltas.push_back(-data_point->z * std::log(model_value));
    }
  }

  while (deltas.size() > 2) {
    std::vector<mydouble> temp_sum;
    temp_sum.reserve(deltas.size() / 2 + 1);
    if (deltas.size() % 2 == 0) {
      for (unsigned int i = 0; i < deltas.size(); i = i + 2) {
        temp_sum.push_back(deltas[i] + deltas[i + 1]);
      }
    } else {
      for (unsigned int i = 0; i < deltas.size() - 1; i = i + 2) {
        temp_sum.push_back(deltas[i] + deltas[i + 1]);
      }
      temp_sum.push_back(deltas.back());
    }
    deltas = temp_sum;
  }

  mydouble sum_value(0.0);
  if (deltas.size() == 1)
    sum_value = deltas[0];
  if (deltas.size() == 2)
    sum_value = deltas[0] + deltas[1];

  return sum_value;
}
