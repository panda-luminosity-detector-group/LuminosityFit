/*
 * PolynomialModel1D.cxx
 *
 *  Created on: Apr 5, 2013
 *      Author: steve
 */

#include "PolynomialModel1D.h"
#include "core/ModelPar.h"

#include "stdio.h"
#include <cmath>
#include <limits>

PolynomialModel1D::PolynomialModel1D(std::string name_, unsigned int order_)
    : Model1D(name_) {
  
  order = order_;
  initModelParameters();
}

PolynomialModel1D::~PolynomialModel1D() {
  
}

void PolynomialModel1D::initModelParameters() {
  poly_factors.clear();
  for (unsigned int i = 0; i < order + 1; i++) {
    char c[30];
    sprintf(c, "poly_poly_factor_%u", i);
    poly_factors.push_back(getModelParameterSet().addModelParameter(c));
  }
}

mydouble PolynomialModel1D::eval(const mydouble *x) const {
  mydouble val = 0.0;
  for (unsigned int i = 0; i < poly_factors.size(); i++) {
    val += poly_factors[i]->getValue() * std::pow(x[0], static_cast<int>(i));
  }
  return val;
}

void PolynomialModel1D::updateDomain() {
  setDomain(std::numeric_limits<mydouble>::min(),
            std::numeric_limits<mydouble>::max());
}
