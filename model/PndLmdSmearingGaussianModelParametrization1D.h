/*
 * PndLmdSmearingGaussianModelParametrization1D.h
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#ifndef PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION1D_H_
#define PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION1D_H_

#include "SharedPtr.h"

class Model1D;
class ModelPar;
namespace LumiFit {
  class PndLmdFitModelOptions;
}

class PndLmdSmearingGaussianModelParametrization1D {
private:
	shared_ptr<ModelPar> p_lab;
public:
	PndLmdSmearingGaussianModelParametrization1D(shared_ptr<Model1D> model,
			const LumiFit::PndLmdFitModelOptions& model_options);
	virtual ~PndLmdSmearingGaussianModelParametrization1D();
};

#endif /* PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION1D_H_ */
