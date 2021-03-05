#ifndef PNDLMDFASTDPMANGMODEL2D_H_
#define PNDLMDFASTDPMANGMODEL2D_H_

#include "PndLmdDPMAngModel1D.h"
#include "core/Model2D.h"

class PndLmdFastDPMAngModel2D : public Model2D {
  mydouble one_over_two_pi;

  std::shared_ptr<Model> dpm_model_1d;

  std::shared_ptr<ModelPar> tilt_x;
  std::shared_ptr<ModelPar> tilt_y;

  mydouble calculateThetaFromTiltedSystem(const mydouble theta,
                                          const mydouble phi) const;

  mydouble calculateJacobianDeterminant(const mydouble theta,
                                        const mydouble phi) const;

public:
  PndLmdFastDPMAngModel2D(std::string name_,
                          std::shared_ptr<PndLmdDPMAngModel1D> dpm_model_1d_);
  virtual ~PndLmdFastDPMAngModel2D();

  virtual void initModelParameters();

  mydouble eval(const mydouble *x) const;

  virtual void updateDomain();
};

#endif /* PNDLMDFASTDPMANGMODEL2D_H_ */
