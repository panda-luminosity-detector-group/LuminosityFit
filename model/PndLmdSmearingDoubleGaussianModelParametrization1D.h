/*
 * PndLmdSmearingDoubleGaussianModelParametrization1D.h
 *
 *  Created on: Mar 28, 2013
 *      Author: steve
 */

#ifndef PNDLMDSMEARINGDOUBLEGAUSSIANMODELPARAMETRIZATION1D_H_
#define PNDLMDSMEARINGDOUBLEGAUSSIANMODELPARAMETRIZATION1D_H_

#include <memory>

class Model1D;
class ModelPar;

class PndLmdSmearingDoubleGaussianModelParametrization1D {
private:
  std::shared_ptr<ModelPar> p_lab;
public:
  PndLmdSmearingDoubleGaussianModelParametrization1D(std::shared_ptr<Model1D> model);
  virtual ~PndLmdSmearingDoubleGaussianModelParametrization1D();
};

#endif /* PNDLMDSMEARINGDOUBLEGAUSSIANMODELPARAMETRIZATION1D_H_ */
