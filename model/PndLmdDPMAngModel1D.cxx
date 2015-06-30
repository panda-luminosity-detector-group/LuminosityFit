/*
 * PndLmdDPMAngModel1D.cxx
 *
 *  Created on: Mar 7, 2013
 *      Author: steve
 */

#include "PndLmdDPMAngModel1D.h"

#include "TMath.h"

PndLmdDPMAngModel1D::PndLmdDPMAngModel1D(std::string name_,
		LumiFit::DPMElasticParts elastic_type_) :
		PndLmdDPMMTModel1D(name_, elastic_type_) {

	initModelParameters();
}

PndLmdDPMAngModel1D::~PndLmdDPMAngModel1D() {
	// TODO Auto-generated destructor stub
}

double PndLmdDPMAngModel1D::getMomentumTransferFromTheta(
		const double theta) const {
	// read lmd note/tdr for a derivation of this formula
	double signum = -1.0;
	double denominator = p_lab->getValue() * cos(theta)
			- beta_lab_cms->getValue() * E_lab->getValue();
	if (0.0 > denominator)
		signum = 1.0;

	double t = -2.0 * pcm2->getValue()
			* (1.0
					+ signum
							/ sqrt(
									1
											+ pow(
													p_lab->getValue() * sin(theta)
															/ (gamma->getValue() * denominator), 2.0)));
	return t;
}

double PndLmdDPMAngModel1D::getThetaMomentumTransferJacobian(
		const double theta) const {
	//numerical derivate calculation
	//is quite accurate with relatively easy implementation
	//via f (x) ≈ [f(x + h) − f(x − h)] / 2h
	//important!: pick h appropriately
	double e_m = 1.0 * 1e-16; // machine precision
	double h = pow(e_m, 0.33) * theta;

	return TMath::Abs(
			(getMomentumTransferFromTheta(theta + h)
					- getMomentumTransferFromTheta(theta - h)) / (2 * h));
}

double PndLmdDPMAngModel1D::eval(const double *x) const {
	double theta = x[0];
	double t = getMomentumTransferFromTheta(theta);
	double jaco = getThetaMomentumTransferJacobian(theta);
	return PndLmdDPMMTModel1D::eval(&t) * jaco;
}

void PndLmdDPMAngModel1D::updateDomain() {
	setDomain(0, TMath::Pi());
}
