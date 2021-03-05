/*
 * PndLmdSmearingGaussianModelParametrization1D.h
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#ifndef PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION1D_H_
#define PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION1D_H_

#include <memory>

class Model1D;
class ModelPar;
namespace LumiFit {
class PndLmdFitModelOptions;
}

class PndLmdSmearingGaussianModelParametrization1D {
private:
  std::shared_ptr<ModelPar> p_lab;

public:
  PndLmdSmearingGaussianModelParametrization1D(
      std::shared_ptr<Model1D> model,
      const LumiFit::PndLmdFitModelOptions &model_options);
  virtual ~PndLmdSmearingGaussianModelParametrization1D();
};

#endif /* PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION1D_H_ */
