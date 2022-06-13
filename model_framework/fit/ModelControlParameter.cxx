/*
 * ModelControlParameter.cxx
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#include "ModelControlParameter.h"

ModelControlParameter::ModelControlParameter() {
  
}

ModelControlParameter::~ModelControlParameter() {
  
}

vector<ModelStructs::minimization_parameter> &
ModelControlParameter::getParameterList() {
  return parameters;
}
