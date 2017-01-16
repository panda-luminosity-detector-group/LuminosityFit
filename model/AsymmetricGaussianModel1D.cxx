/*
 * AsymmetricGaussianModel.cxx
 *
 *  Created on: Dec 17, 2012
 *      Author: steve
 */

#include "AsymmetricGaussianModel1D.h"

#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

AsymmetricGaussianModel1D::AsymmetricGaussianModel1D(std::string name_) :
		Model1D(name_) {
	initModelParameters();
	num_sigmas = 5.0;
}

AsymmetricGaussianModel1D::~AsymmetricGaussianModel1D() {

}

void AsymmetricGaussianModel1D::initModelParameters() {
	asymm_gauss_sigma_left = getModelParameterSet().addModelParameter(
			"asymm_gauss_sigma_left");
	asymm_gauss_sigma_right = getModelParameterSet().addModelParameter(
			"asymm_gauss_sigma_right");
	asymm_gauss_mean = getModelParameterSet().addModelParameter(
			"asymm_gauss_mean");
	// as default the amplitude of the gauss is set to 1,
	// which give a unit integral
	asymm_gauss_amplitude = getModelParameterSet().addModelParameter(
			"asymm_gauss_amplitude");
	asymm_gauss_amplitude->setValue(1.0);
	asymm_gauss_amplitude->setParameterFixed(true);
}

mydouble AsymmetricGaussianModel1D::eval(const double *x) const {
	double value = 0.0;
	// left side
	if (x[0] < asymm_gauss_mean->getValue()) {
		value = asymm_gauss_amplitude->getValue()
				* exp(
						0.5
								* -pow(
										(x[0] - asymm_gauss_mean->getValue())
												/ asymm_gauss_sigma_left->getValue(), 2.0))
				/ ((asymm_gauss_sigma_left->getValue()
						+ asymm_gauss_sigma_right->getValue()) * sqrt(2.0 * M_PI));
	} else { // right side
		value = asymm_gauss_amplitude->getValue()
				* exp(
						0.5
								* -pow(
										(x[0] - asymm_gauss_mean->getValue())
												/ asymm_gauss_sigma_right->getValue(), 2.0))
				/ ((asymm_gauss_sigma_left->getValue()
						+ asymm_gauss_sigma_right->getValue()) * sqrt(2.0 * M_PI));
	}
	return 2.0*value;
}

void AsymmetricGaussianModel1D::updateDomain() {
	setDomain(
			-num_sigmas * asymm_gauss_sigma_left->getValue()
					+ asymm_gauss_mean->getValue(),
			num_sigmas * asymm_gauss_sigma_right->getValue()
					+ asymm_gauss_mean->getValue());
}
