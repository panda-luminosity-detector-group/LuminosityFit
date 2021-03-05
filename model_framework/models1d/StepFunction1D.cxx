/*
 * StepFunction.cxx
 *
 *  Created on: Jun 14, 2013
 *      Author: steve
 */

#include "StepFunction1D.h"

#include <limits>

StepFunction1D::StepFunction1D(std::string name_, bool falling_edge_)
    : Model1D(name_), falling_edge(falling_edge_) {
  initModelParameters();
}

StepFunction1D::~StepFunction1D() {
  // TODO Auto-generated destructor stub
}

mydouble StepFunction1D::eval(const mydouble *x) const {
  if (falling_edge) {
    if (x[0] < edge->getValue())
      return amplitude->getValue();
  } else {
    if (x[0] > edge->getValue())
      return amplitude->getValue();
  }
  return 0.0;
}

void StepFunction1D::initModelParameters() {
  amplitude = getModelParameterSet().addModelParameter("amplitude");
  amplitude->setValue(1.0);
  amplitude->setParameterFixed(true);
  edge = getModelParameterSet().addModelParameter("edge");
}

void StepFunction1D::updateDomain() {
  if (falling_edge)
    setDomain(std::numeric_limits<mydouble>::min(), edge->getValue());
  else
    setDomain(edge->getValue(), std::numeric_limits<mydouble>::max());
}
