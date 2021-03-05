/*
 * ModelParameterHandler.h
 *
 *  Created on: Mar 19, 2013
 *      Author: steve
 */

#ifndef MODELPARAMETERHANDLER_H_
#define MODELPARAMETERHANDLER_H_

#include "ModelParSet.h"
#include "Parametrization.h"
#include "ParametrizationModel.h"
#include "ParametrizationProxy.h"
#include "fit/ModelFitResult.h"

#include <map>
#include <set>

class ModelParameterHandler {
private:
  /**
   * The parameter set of the model.
   */
  ModelParSet model_par_set;

  /**
   * Map of ParametrizationProxys, which is either a Parametrization or a
   * ParametrizationModel, for every ModelPar of this Model that is being
   * parametrized.
   * -Parametrizations that are called at each fit iteration step to
   * update parameters due to connections to free parameters that have changed
   * -ParametrizationModels are called at each evaluation as the model
   * parameters are a dependent on the domain variables as well
   */
  std::map<const std::shared_ptr<ModelPar>, ParametrizationProxy>
      parametrizations;

public:
  ModelParameterHandler(std::string model_name_);
  virtual ~ModelParameterHandler();

  ModelParSet &getModelParameterSet();

  int checkParametrizations() const;

  bool hasParametrizationModels() const;

  int checkParameters();

  void registerUpdaters();

  void reinitModelParametrizations();

  void
  registerParametrization(std::shared_ptr<ModelPar> model_par,
                          std::shared_ptr<Parametrization> parametrization);

  void
  registerParametrizations(ModelParSet &model_par_set_,
                           std::shared_ptr<Parametrization> parametrization);

  void registerParametrizationModel(
      std::shared_ptr<ModelPar> model_par,
      std::shared_ptr<ParametrizationModel> parametrization_model);

  ParametrizationProxy
  getParametrizationProxyForModelParameter(std::string name_);

  void executeParametrizationModels(const mydouble *x);

  /**
   * This method updates the model parameters that are set via
   * parametrizations, and is automatically called from the #Model::updateModel
   * function.
   */
  void updateModelParameters();

  void initModelParametersFromFitResult(const ModelFitResult &fit_result);
};

#endif /* MODELPARAMETERHANDLER_H_ */
