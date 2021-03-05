/*
 * Model2D.h
 *
 *  Created on: Jan 16, 2013
 *      Author: steve
 */

#ifndef MODEL2D_H_
#define MODEL2D_H_

#include "core/Model.h"

class IntegralStrategy2D;

class Model2D : public Model {
private:
  std::pair<mydouble, mydouble> var1_domain_bounds;
  std::pair<mydouble, mydouble> var2_domain_bounds;

  std::shared_ptr<IntegralStrategy2D> integral_strategy;

public:
  Model2D(std::string name_);
  virtual ~Model2D();

  mydouble getVar1DomainRange();
  mydouble getVar1DomainLowerBound();
  mydouble getVar2DomainRange();
  mydouble getVar2DomainLowerBound();

  void setVar1Domain(mydouble lower_bound, mydouble upper_bound);
  void setVar2Domain(mydouble lower_bound, mydouble upper_bound);

  void
  setIntegralStrategy(std::shared_ptr<IntegralStrategy2D> integral_strategy_);

  mydouble Integral(const std::vector<DataStructs::DimensionRange> &ranges,
                    mydouble precision);
};

#endif /* MODEL2D_H_ */
