/*
 * ModelMinimizationControlParameter.cxx
 *
 *  Created on: Jun 6, 2013
 *      Author: steve
 */

#include "ModelMinimizationControlParameter.h"
#include "core/Model.h"

ModelMinimizationControlParameter::ModelMinimizationControlParameter()
    : model() {}

ModelMinimizationControlParameter::~ModelMinimizationControlParameter() {
  
}

mydouble ModelMinimizationControlParameter::evaluate(const mydouble *pars) {
  return model->eval(pars);
}
