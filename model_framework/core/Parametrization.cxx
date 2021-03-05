/*
 * Parametrization.cxx
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#include "Parametrization.h"

Parametrization::Parametrization(ModelParSet &model_par_set_)
    : model_par_set(model_par_set_), dependency_parameters() {}

Parametrization::~Parametrization() {
  // TODO Auto-generated destructor stub
}

void Parametrization::init() {
  dependency_parameters.clear();
  initParameters();
}

int Parametrization::check() const {
  for (unsigned int i = 0; i < dependency_parameters.size(); i++) {
    if (!dependency_parameters[i]->isSet())
      return 1;
  }
  return 0;
}
