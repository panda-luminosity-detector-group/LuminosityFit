/*
 * SimpleIntegralStrategy2D.h
 *
 *  Created on: Sep 26, 2014
 *      Author: steve
 */

#ifndef SIMPLEINTEGRALSTRATEGY2D_H_
#define SIMPLEINTEGRALSTRATEGY2D_H_

#include <operators2d/integration/IntegralStrategy2D.h>

class Model2D;

class SimpleIntegralStrategy2D: public IntegralStrategy2D {
  unsigned int max_grid_constant;
  unsigned int used_grid_constant;
public:
  SimpleIntegralStrategy2D();
  virtual ~SimpleIntegralStrategy2D();

  void setUsedEvaluationGridConstant(unsigned int used_grid_constant_);
  void setMaximumEvaluationGridConstant(unsigned int max_grid_constant_);

  unsigned int determineOptimalCallNumber(Model2D *model2d,
      const std::vector<DataStructs::DimensionRange> &ranges, double precision);

  mydouble Integral(Model2D *model2d,
      const std::vector<DataStructs::DimensionRange> &ranges, mydouble precision);
};

#endif /* SIMPLEINTEGRALSTRATEGY2D_H_ */
