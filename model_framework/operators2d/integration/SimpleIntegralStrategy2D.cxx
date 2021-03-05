/*
 * SimpleIntegralStrategy2D.cxx
 *
 *  Created on: Sep 26, 2014
 *      Author: steve
 */

#include <cmath>
#include <core/Model2D.h>
#include <iostream>
#include <operators2d/integration/SimpleIntegralStrategy2D.h>

SimpleIntegralStrategy2D::SimpleIntegralStrategy2D() {
  // TODO Auto-generated constructor stub
}

SimpleIntegralStrategy2D::~SimpleIntegralStrategy2D() {
  // TODO Auto-generated destructor stub
}

void SimpleIntegralStrategy2D::setUsedEvaluationGridConstant(
    unsigned int used_grid_constant_) {
  used_grid_constant = used_grid_constant_;
}
void SimpleIntegralStrategy2D::setMaximumEvaluationGridConstant(
    unsigned int max_grid_constant_) {
  max_grid_constant = max_grid_constant_;
}

unsigned int SimpleIntegralStrategy2D::determineOptimalCallNumber(
    Model2D *model2d, const std::vector<DataStructs::DimensionRange> &ranges,
    double precision) {
  used_grid_constant = 1;

  mydouble result, result_higher;
  do {
    result = Integral(model2d, ranges, precision);
    used_grid_constant *= 2;
    result_higher = Integral(model2d, ranges, precision);
    // std::cout<<result<< " and "<<result_higher<<std::endl;
  } while (std::fabs(result - result_higher) / result_higher > precision);

  return used_grid_constant / 2;
}

mydouble SimpleIntegralStrategy2D::Integral(
    Model2D *model2d, const std::vector<DataStructs::DimensionRange> &ranges,
    mydouble precision) {
  mydouble result = 0.0;
  mydouble x[2];

  mydouble wx =
      (ranges[0].range_high - ranges[0].range_low) / used_grid_constant;
  mydouble wy =
      (ranges[1].range_high - ranges[1].range_low) / used_grid_constant;

  mydouble half(0.5);
  for (unsigned int ix = 0; ix < used_grid_constant; ++ix) {
    for (unsigned int iy = 0; iy < used_grid_constant; ++iy) {
      x[0] = ranges[0].range_low + wx * (half + ix);
      x[1] = ranges[1].range_low + wy * (half + iy);
      mydouble tempresult = model2d->evaluate(x);
      result += tempresult;
    }
  }
  result = result * wx * wy;

  return result;
}
