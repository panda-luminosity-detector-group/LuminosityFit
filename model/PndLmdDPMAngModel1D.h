/*
 * PndLmdDPMAngModel1D.h
 *
 *  Created on: Mar 7, 2013
 *      Author: steve
 */

#ifndef PNDLMDDPMANGMODEL1D_H_
#define PNDLMDDPMANGMODEL1D_H_

#include "PndLmdDPMMTModel1D.h"

class PndLmdDPMAngModel1D: public PndLmdDPMMTModel1D {
// function pointer used to switch between different algorithms for interpolation
  typedef double (PndLmdDPMAngModel1D::*trans_function)(const double theta) const;

  trans_function trafo_func;

public:
    PndLmdDPMAngModel1D(std::string name_, LumiFit::DPMElasticParts elastic_type_, LumiFit::TransformationOption trafo_type);
    virtual ~PndLmdDPMAngModel1D();

    double getMomentumTransferFromThetaCorrect(const double theta) const;
    double getMomentumTransferFromThetaApprox(const double theta) const;

    double getMomentumTransferFromTheta(const double theta) const;
    
    double getThetaMomentumTransferJacobian(const double theta) const;

    /**
     * @param x theta and phi value, which are stored in x[0] and x[1]
     * @param par six parameters of the fit. par[0] is the luminosity which is a free parameter of the fit
     * par[1-3] are the parameters of the DPM model and par[4-5] are theta and phi bin sizes of the data
     * @returns cross section value for given theta and phi value, which are stored in x[0] and x[1] and the parameters:
     * par */
    double eval(const double *x) const;

    virtual void updateDomain();
};

#endif /* PNDLMDDPMANGMODEL1D_H_ */
