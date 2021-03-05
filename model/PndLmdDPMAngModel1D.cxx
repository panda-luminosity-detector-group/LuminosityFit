/*
 * PndLmdDPMAngModel1D.cxx
 *
 *  Created on: Mar 7, 2013
 *      Author: steve
 */

#include "PndLmdDPMAngModel1D.h"

#include <cmath>

#include "TLorentzVector.h"
#include "TMath.h"

PndLmdDPMAngModel1D::PndLmdDPMAngModel1D(
    std::string name_, LumiFit::DPMElasticParts elastic_type_,
    LumiFit::TransformationOption trafo_type)
    : PndLmdDPMMTModel1D(name_, elastic_type_) {

  initModelParameters();

  if (trafo_type == LumiFit::CORRECT) {
    trafo_func = &PndLmdDPMAngModel1D::getMomentumTransferFromThetaCorrect;
  } else {
    trafo_func = &PndLmdDPMAngModel1D::getMomentumTransferFromThetaApprox;
  }
}

PndLmdDPMAngModel1D::~PndLmdDPMAngModel1D() {
  // TODO Auto-generated destructor stub
}

mydouble PndLmdDPMAngModel1D::getMomentumTransferFromThetaCorrect(
    const mydouble theta) const {
  TLorentzVector before(0, 0, p_lab->getValue(), E_lab->getValue());
  TLorentzVector after(before);
  after.SetTheta(theta);

  // after.Boost(0.0, 0.0, -beta_lab_cms->getValue());
  return (after - before).M2();
}

mydouble PndLmdDPMAngModel1D::getMomentumTransferFromThetaApprox(
    const mydouble theta) const {
  // read lmd note/tdr for a derivation of this formula
  mydouble signum = -1.0L;
  mydouble denominator = p_lab->getValue() * cos(theta) -
                         beta_lab_cms->getValue() * E_lab->getValue();
  if (0.0 > denominator)
    signum = 1.0L;

  mydouble temp(p_lab->getValue() * sin(theta) /
                (gamma->getValue() * denominator));
  mydouble t = -2.0L * pcm2->getValue() *
               (1.0L + signum / std::sqrt(1.0L + temp * temp));
  return t;
}

mydouble
PndLmdDPMAngModel1D::getMomentumTransferFromTheta(const mydouble theta) const {
  return (this->*trafo_func)(theta);
}

mydouble PndLmdDPMAngModel1D::getThetaMomentumTransferJacobian(
    const mydouble theta) const {
  // numerical derivate calculation
  // is quite accurate with relatively easy implementation
  // via f (x) ≈ [f(x + h) − f(x − h)] / 2h
  // important!: pick h appropriately
  mydouble e_m = 1.0L * 1e-16L; // machine precision
  mydouble h = std::pow(e_m, 0.33) * theta;

  return std::fabs(
      ((this->*trafo_func)(theta + h) - (this->*trafo_func)(theta - h)) /
      (h * 2.0));
}

mydouble PndLmdDPMAngModel1D::eval(const mydouble *x) const {
  // return luminosity->getValue();

  mydouble t = (this->*trafo_func)(x[0]);
  mydouble jaco = getThetaMomentumTransferJacobian(x[0]);
  return PndLmdDPMMTModel1D::eval(&t) * jaco;
}

void PndLmdDPMAngModel1D::updateDomain() { setDomain(0, TMath::Pi()); }
