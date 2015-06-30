/*
 * PndLmdLumiFitResult.cxx
 *
 *  Created on: Jun 28, 2012
 *      Author: steve
 */

#include "PndLmdLumiFitResult.h"

ClassImp(PndLmdLumiFitResult)

PndLmdLumiFitResult::PndLmdLumiFitResult() :
		luminosity_sys_err(0) {
}

PndLmdLumiFitResult::~PndLmdLumiFitResult() {
}

double PndLmdLumiFitResult::getLuminosity() const {
	return model_fit_result.getFitParameter("luminosity").value;
}
double PndLmdLumiFitResult::getLuminositySysError() const {
	return luminosity_sys_err;
}
double PndLmdLumiFitResult::getLuminosityStatError() const {
	return model_fit_result.getFitParameter("luminosity").error;
}
double PndLmdLumiFitResult::getLuminosityError() const {
	return getLuminositySysError() + getLuminosityStatError();
}

const ModelFitResult& PndLmdLumiFitResult::getModelFitResult() const {
	return model_fit_result;
}

double PndLmdLumiFitResult::getRedChiSquare() const {
	return model_fit_result.getFinalEstimatorValue() / model_fit_result.getNDF();
}

void PndLmdLumiFitResult::setLuminositySysError(double luminosity_sys_err_) {
	luminosity_sys_err = luminosity_sys_err_;
}

void PndLmdLumiFitResult::setModelFitResult(const ModelFitResult &fit_result) {
	model_fit_result = fit_result;
}
