/*
 * ModelDomainVar.cxx
 *
 *  Created on: Jul 2, 2013
 *      Author: steve
 */

#include "ModelDomainVar.h"

ModelDomainVar::ModelDomainVar() {
  
}

ModelDomainVar::~ModelDomainVar() {
  
}

double ModelDomainVar::getDomainRange() {
  return domain_bounds.second - domain_bounds.first;
}

double ModelDomainVar::getDomainLowerBound() { return domain_bounds.first; }

const std::pair<double, double> &ModelDomainVar::getDomain() const {
  return domain_bounds;
}

void ModelDomainVar::setDomain(double lower_bound, double upper_bound) {
  domain_bounds.first = lower_bound;
  domain_bounds.second = upper_bound;
}
