/*
 * Model1D.cxx
 *
 *  Created on: Jan 16, 2013
 *      Author: steve
 */

#include "Model1D.h"
#include "operators1d/integration/IntegralStrategyGSL1D.h"

Model1D::Model1D(std::string name_) :
		Model(name_, 1), domain_bounds(), integral_strategy(new IntegralStrategyGSL1D()) {
}

Model1D::~Model1D() {
	// TODO Auto-generated destructor stub
}

mydouble Model1D::Integral(const std::vector<DataStructs::DimensionRange> &ranges,
		mydouble precision) {
	return integral_strategy->Integral(this, ranges[0].range_low,
			ranges[0].range_high, precision);
}

mydouble Model1D::getDomainRange() {
	return domain_bounds.second - domain_bounds.first;
}

mydouble Model1D::getDomainLowerBound() {
	return domain_bounds.first;
}

const std::pair<mydouble, mydouble>& Model1D::getDomain() const {
	return domain_bounds;
}

void Model1D::setDomain(mydouble lower_bound, mydouble upper_bound) {
	domain_bounds.first = lower_bound;
	domain_bounds.second = upper_bound;
}
