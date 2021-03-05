/*
 * Model2D.cxx
 *
 *  Created on: Jan 16, 2013
 *      Author: steve
 */

#include "Model2D.h"

#include "operators2d/integration/IntegralStrategyGSL2D.h"

Model2D::Model2D(std::string name_)
    : Model(name_, 2), var1_domain_bounds(), var2_domain_bounds(),
      integral_strategy(new IntegralStrategyGSL2D()) {}

Model2D::~Model2D() {
  // TODO Auto-generated destructor stub
}

mydouble Model2D::getVar1DomainRange() {
  return var1_domain_bounds.second - var1_domain_bounds.first;
}
mydouble Model2D::getVar2DomainRange() {
  return var2_domain_bounds.second - var2_domain_bounds.first;
}

mydouble Model2D::getVar1DomainLowerBound() { return var1_domain_bounds.first; }
mydouble Model2D::getVar2DomainLowerBound() { return var2_domain_bounds.first; }
void Model2D::setVar1Domain(mydouble lower_bound, mydouble upper_bound) {
  var1_domain_bounds.first = lower_bound;
  var1_domain_bounds.second = upper_bound;
}
void Model2D::setVar2Domain(mydouble lower_bound, mydouble upper_bound) {
  var2_domain_bounds.first = lower_bound;
  var2_domain_bounds.second = upper_bound;
}

void Model2D::setIntegralStrategy(
    std::shared_ptr<IntegralStrategy2D> integral_strategy_) {
  integral_strategy = integral_strategy_;
}

mydouble
Model2D::Integral(const std::vector<DataStructs::DimensionRange> &ranges,
                  mydouble precision) {
  return integral_strategy->Integral(this, ranges, precision);
}
