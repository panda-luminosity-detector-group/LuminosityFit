/*
 * ModelDomainVar.h
 *
 *  Created on: Jul 2, 2013
 *      Author: steve
 */

#ifndef MODELDOMAINVAR_H_
#define MODELDOMAINVAR_H_

#include <utility>

class ModelDomainVar {
	private:
  std::pair<double, double> domain_bounds;
public:
	ModelDomainVar();
	virtual ~ModelDomainVar();

  /**
   * Returns a vector of value pairs representing the domain of the model. The
   * first value of each pair is the lower, the second the upper bound of the
   * domain. Each entry in the vector represents one dimension.
   */
  const std::pair<double, double>& getDomain() const;

  /**
   * Sets the domain of the model. The domain is defined to be non-zero within
   * the domain and zero outside. Hence this setter should be used with care.
   * @param lower_bound is the lower bound of the domain
   * @param upper_bound is the upper bound of the domain
   */
  void setDomain(double lower_bound, double upper_bound);

  double getDomainRange();
  double getDomainLowerBound();
};

#endif /* MODELDOMAINVAR_H_ */
