/*
 * PndLmdDPMAngModel1D.h
 *
 *  Created on: Mar 7, 2013
 *      Author: steve
 */

#ifndef PNDLMDDPMANGMODEL1D_H_
#define PNDLMDDPMANGMODEL1D_H_

#include "PndLmdDPMMTModel1D.h"

class PndLmdDPMAngModel1D : public PndLmdDPMMTModel1D {
  // function pointer used to switch between different algorithms for
  // interpolation
  typedef mydouble (PndLmdDPMAngModel1D::*trans_function)(
      const mydouble theta) const;

  trans_function trafo_func;

public:
  PndLmdDPMAngModel1D(std::string name_, LumiFit::DPMElasticParts elastic_type_,
                      LumiFit::TransformationOption trafo_type);
  virtual ~PndLmdDPMAngModel1D();

  mydouble getMomentumTransferFromThetaCorrect(const mydouble theta) const;
  mydouble getMomentumTransferFromThetaApprox(const mydouble theta) const;

  mydouble getMomentumTransferFromTheta(const mydouble theta) const;

  mydouble getThetaMomentumTransferJacobian(const mydouble theta) const;

  /**
   * @param x theta and phi value, which are stored in x[0] and x[1]
   * @param par six parameters of the fit. par[0] is the luminosity which is a
   * free parameter of the fit par[1-3] are the parameters of the DPM model and
   * par[4-5] are theta and phi bin sizes of the data
   * @returns cross section value for given theta and phi value, which are
   * stored in x[0] and x[1] and the parameters: par */
  mydouble eval(const mydouble *x) const;

  virtual void updateDomain();
};

#endif /* PNDLMDDPMANGMODEL1D_H_ */
