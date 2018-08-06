/*
 * ModelControlParameter.h
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#ifndef MODELCONTROLPARAMETER_H_
#define MODELCONTROLPARAMETER_H_

#include "core/ModelStructs.h"
#include "ProjectWideSettings.h"

#include <vector>

using std::vector;

class ModelControlParameter {
private:
	vector<ModelStructs::minimization_parameter> parameters;
public:
	ModelControlParameter();
	virtual ~ModelControlParameter();

	virtual mydouble evaluate(const mydouble *pars) =0;

	vector<ModelStructs::minimization_parameter>& getParameterList();
};

#endif /* MODELCONTROLPARAMETER_H_ */
