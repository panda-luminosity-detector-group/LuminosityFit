/*
 * PndLmdDPMAngModel2D.h
 *
 *  Created on: Jan 17, 2013
 *      Author: steve
 */

#ifndef PNDLMDDPMANGMODEL2D_H_
#define PNDLMDDPMANGMODEL2D_H_

#include "core/Model2D.h"
#include "PndLmdDPMAngModel1D.h"

class PndLmdDPMAngModel2D: public Model2D {
	shared_ptr<Model> dpm_model_1d;

	shared_ptr<ModelPar> tilt_x;
	shared_ptr<ModelPar> tilt_y;

	std::pair<mydouble, mydouble> calculateThetaFromTiltedSystem(const mydouble theta,
			const mydouble phi) const;

	mydouble calculateJacobianDeterminant(const mydouble theta,
			const mydouble phi) const;

public:
	PndLmdDPMAngModel2D(std::string name_,
			shared_ptr<PndLmdDPMAngModel1D> dpm_model_1d_);
	virtual ~PndLmdDPMAngModel2D();

	virtual void initModelParameters();

	mydouble eval(const mydouble *x) const;

	virtual void updateDomain();
};

#endif /* PNDLMDDPMANGMODEL2D_H_ */
