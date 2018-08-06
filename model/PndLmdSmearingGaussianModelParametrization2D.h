/*
 * PndLmdSmearingGaussianModelParametrization2D.h
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#ifndef PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION2D_H_
#define PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION2D_H_

#include <memory>

class Model2D;
class ModelPar;
namespace LumiFit {
  class PndLmdFitModelOptions;
}

class PndLmdSmearingGaussianModelParametrization2D {
private:
	std::shared_ptr<ModelPar> p_lab;
public:
	PndLmdSmearingGaussianModelParametrization2D(std::shared_ptr<Model2D> model,
			const LumiFit::PndLmdFitModelOptions& model_options);
	virtual ~PndLmdSmearingGaussianModelParametrization2D();
};

#endif /* PNDLMDSMEARINGGAUSSIANMODELPARAMETRIZATION2D_H_ */
