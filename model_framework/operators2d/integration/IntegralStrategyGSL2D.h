#ifndef INTEGRALSTRATEGYGSL2D_H_
#define INTEGRALSTRATEGYGSL2D_H_

#include "IntegralStrategy2D.h"

#include "core/Model2D.h"

#include <gsl/gsl_math.h>
#include <gsl/gsl_monte.h>
#include <gsl/gsl_monte_vegas.h>

extern thread_local Model2D *current_model;

class IntegralStrategyGSL2D: public IntegralStrategy2D {
private:
  const gsl_rng_type *T;
  size_t start_calls;
  size_t max_calls;

  static double gsl_func_wrapper(double *x, size_t dim, void *params) {
    Model2D *current_model_temp = current_model;
    mydouble xtemp[2];
    xtemp[0] = (mydouble)x[0];
    xtemp[1] = (mydouble)x[1];
    mydouble value = current_model->eval(xtemp);
    current_model = current_model_temp;
    return (double)value;
  }

public:
  IntegralStrategyGSL2D(unsigned int start_calls_ = 500,
      unsigned int max_calls_ = 100000);
  virtual ~IntegralStrategyGSL2D();

  void setStartNumberOfFunctionEvaluations(unsigned int start_calls_);
  void setMaximumNumberOfFunctionEvaluations(unsigned int max_calls_);

  unsigned int determineOptimalCallNumber(Model2D *model2d,
      const std::vector<DataStructs::DimensionRange> &ranges, mydouble precision);

  mydouble Integral(Model2D *model2d,
      const std::vector<DataStructs::DimensionRange> &ranges, mydouble precision);
};
#endif /* INTEGRALSTRATEGYGSL2D_H_ */
