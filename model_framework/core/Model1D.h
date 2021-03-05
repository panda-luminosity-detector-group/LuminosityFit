/*
 * Model1D.h
 *
 *  Created on: Jan 16, 2013
 *      Author: steve
 */

#ifndef MODEL1D_H_
#define MODEL1D_H_

#include "Model.h"

class IntegralStrategy1D;

class Model1D : public Model {
private:
  std::pair<mydouble, mydouble> domain_bounds;

  std::shared_ptr<IntegralStrategy1D> integral_strategy;

public:
  Model1D(std::string name_);
  virtual ~Model1D();

  mydouble Integral(const std::vector<DataStructs::DimensionRange> &ranges,
                    mydouble precision);
  mydouble getDomainRange();
  mydouble getDomainLowerBound();
  const std::pair<mydouble, mydouble> &getDomain() const;
  void setDomain(mydouble lower_bound, mydouble upper_bound);
};

#endif /* MODEL1D_H_ */
