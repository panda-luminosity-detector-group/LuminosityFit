#include "IntegralStrategyGSL2D.h"

#include <cmath>
#include <iostream>

thread_local Model2D *current_model = 0;

IntegralStrategyGSL2D::IntegralStrategyGSL2D(unsigned int start_calls_,
                                             unsigned int max_calls_)
    : start_calls(start_calls_), max_calls(max_calls_), T(gsl_rng_default) {
  gsl_rng_env_setup();
}

IntegralStrategyGSL2D::~IntegralStrategyGSL2D() {}

void IntegralStrategyGSL2D::setStartNumberOfFunctionEvaluations(
    unsigned int start_calls_) {
  start_calls = start_calls_;
}
void IntegralStrategyGSL2D::setMaximumNumberOfFunctionEvaluations(
    unsigned int max_calls_) {
  max_calls = max_calls_;
}

unsigned int IntegralStrategyGSL2D::determineOptimalCallNumber(
    Model2D *model2d, const std::vector<DataStructs::DimensionRange> &ranges,
    mydouble precision) {
  unsigned int num_calls = 2;
  current_model = model2d;
  // gsl_func_wrapper gsl_func(model2d);

  double result, error;

  double xl[2] = {(double)ranges[0].range_low, (double)ranges[1].range_low};
  double xu[2] = {(double)ranges[0].range_high, (double)ranges[1].range_high};

  gsl_rng *r = gsl_rng_alloc(T);
  gsl_monte_vegas_state *s = gsl_monte_vegas_alloc(2);

  gsl_monte_function G = {&gsl_func_wrapper, 2, 0};

  gsl_monte_vegas_init(s);

  while (true) {
    gsl_monte_vegas_integrate(&G, xl, xu, 2, num_calls, r, s, &result, &error);

    if (result == 0 && error == 0)
      break;
    // if (fabs(gsl_monte_vegas_chisq(s) - 1.0) < 0.1)
    //  break;
    if (std::fabs(error / result) < (double)precision)
      break;
    if (max_calls < num_calls)
      break;

    if (num_calls < max_calls)
      num_calls *= 2;
  }

  gsl_monte_vegas_free(s);
  gsl_rng_free(r);

  return num_calls;
}

mydouble IntegralStrategyGSL2D::Integral(
    Model2D *model2d, const std::vector<DataStructs::DimensionRange> &ranges,
    mydouble precision) {
  current_model = model2d;
  // gsl_func_wrapper gsl_func(model2d);

  double result, error;

  double xl[2] = {ranges[0].range_low, ranges[1].range_low};
  double xu[2] = {ranges[0].range_high, ranges[1].range_high};

  gsl_rng *r = gsl_rng_alloc(T);
  gsl_monte_vegas_state *s = gsl_monte_vegas_alloc(2);

  gsl_monte_function G = {&gsl_func_wrapper, 2, 0};

  gsl_monte_vegas_init(s);

  unsigned int num_calls(start_calls);
  while (true) {
    gsl_monte_vegas_integrate(&G, xl, xu, 2, num_calls, r, s, &result, &error);

    break;
    // std::cout << result << " " << error << std::endl;
    // std::cout << calls << std::endl;
    // std::cout << gsl_monte_vegas_chisq(s) << std::endl;

    if (result == 0 && error == 0)
      break;
    // if (fabs(gsl_monte_vegas_chisq(s) - 1.0) < 0.1)
    //	break;
    if (fabs(error / result) < precision)
      break;
    if (max_calls < num_calls)
      break;

    if (num_calls < max_calls)
      num_calls *= 2;
  }

  gsl_monte_vegas_free(s);
  gsl_rng_free(r);

  return result;
}
