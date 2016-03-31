#ifndef PNDLMDFASTDPMANGMODEL2D_H_
#define PNDLMDFASTDPMANGMODEL2D_H_

#include "core/Model2D.h"
#include "PndLmdDPMAngModel1D.h"

class PndLmdFastDPMAngModel2D: public Model2D {
  double one_over_two_pi;

  shared_ptr<Model> dpm_model_1d;

	shared_ptr<ModelPar> tilt_x;
	shared_ptr<ModelPar> tilt_y;

	double calculateThetaFromTiltedSystem(const double theta,
			const double phi) const;

	double calculateJacobianDeterminant(const double theta,
			const double phi) const;

public:
	PndLmdFastDPMAngModel2D(std::string name_,
			shared_ptr<PndLmdDPMAngModel1D> dpm_model_1d_);
	virtual ~PndLmdFastDPMAngModel2D();

	virtual void initModelParameters();

	double eval(const double *x) const;

	virtual void updateDomain();
};

#endif /* PNDLMDFASTDPMANGMODEL2D_H_ */
