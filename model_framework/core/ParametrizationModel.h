/*
 * ParametrizationModel.h
 *
 *  Created on: Apr 2, 2013
 *      Author: steve
 */

#ifndef PARAMETRIZATIONMODEL_H_
#define PARAMETRIZATIONMODEL_H_

#include <memory>
#include "ProjectWideSettings.h"

class Model;
class ModelPar;
class ModelParSet;

/**
 * A #ParametrizationModel is a special type of #Model. It functions as a
 * parametrization to an existing parameter set of another #Model, but
 * describes these parameters as a function of the domain variables of the
 * actual fit function. So it behaves just like a #Model with the addition that
 * it requires a #ModelParSet in the constructor, which will be adjusted.
 */
class ParametrizationModel {
private:
  std::shared_ptr<Model> model;
  std::shared_ptr<ModelPar> model_par;
public:
  ParametrizationModel(std::shared_ptr<Model> model_);
  virtual ~ParametrizationModel();

  void parametrize(const mydouble *x);

  void setModelPar(std::shared_ptr<ModelPar> model_par_);
  const std::shared_ptr<ModelPar> getModelPar() const;

  std::shared_ptr<Model> getModel();
};

#endif /* PARAMETRIZATIONMODEL_H_ */
