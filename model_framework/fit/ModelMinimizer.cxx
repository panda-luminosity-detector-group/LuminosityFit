/*
 * std::shared_ptr<ModelMinimizer>.cxx
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#include "ModelMinimizer.h"

ModelMinimizer::ModelMinimizer() :
    control_parameter() {
}

ModelMinimizer::~ModelMinimizer() {
  // TODO Auto-generated destructor stub
}

std::shared_ptr<ModelControlParameter> ModelMinimizer::getControlParameter() const {
  return control_parameter;
}

void ModelMinimizer::setControlParameter(
    std::shared_ptr<ModelControlParameter> control_parameter_) {
  control_parameter = control_parameter_;
}

void ModelMinimizer::increaseFunctionCallLimit() {
}

int ModelMinimizer::doMinimization() {
  // then apply minimization procedure
  if (control_parameter->getParameterList().size() > 0) {
    return minimize();
  }
  else {
    return -1;
  }
}
