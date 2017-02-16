/*
 * PndLmdBackgroundModel1D.cxx
 *
 *  Created on: Dec 19, 2014
 *      Author: steve
 */

#include <model/PndLmdBackgroundModel1D.h>

#include "models1d/PolynomialModel1D.h"

PndLmdBackgroundModel1D::PndLmdBackgroundModel1D() :
		Model1D("background") {
	polynomial_model.reset(new PolynomialModel1D("polynomial", 3));

	polynomial_model->getModelParameterSet().freeAllModelParameters();
	addModelToList(polynomial_model);
}

PndLmdBackgroundModel1D::~PndLmdBackgroundModel1D() {
}

void PndLmdBackgroundModel1D::initModelParameters() {
}

mydouble PndLmdBackgroundModel1D::eval(const mydouble *x) const {
	return polynomial_model->eval(x);
}

void PndLmdBackgroundModel1D::updateDomain() {
	polynomial_model->updateDomain();
	setDomain(polynomial_model->getDomain().first,
			polynomial_model->getDomain().second);
}
