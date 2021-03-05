/*
 * ModelControlParameter.cxx
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#include "ModelControlParameter.h"

ModelControlParameter::ModelControlParameter() {
  // TODO Auto-generated constructor stub
}

ModelControlParameter::~ModelControlParameter() {
  // TODO Auto-generated destructor stub
}

vector<ModelStructs::minimization_parameter> &
ModelControlParameter::getParameterList() {
  return parameters;
}
