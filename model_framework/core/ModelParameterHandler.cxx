/*
 * ModelParameterHandler.cxx
 *
 *  Created on: Mar 19, 2013
 *      Author: steve
 */

#include "ModelParameterHandler.h"
#include "Model.h"

#include <iostream>

ModelParameterHandler::ModelParameterHandler(std::string model_name_)
    : model_par_set(model_name_), parametrizations() {}

ModelParameterHandler::~ModelParameterHandler() {
  // TODO Auto-generated destructor stub
}

ModelParSet &ModelParameterHandler::getModelParameterSet() {
  return model_par_set;
}

int ModelParameterHandler::checkParametrizations() const {
  int error_code = 0;

  for (std::map<const std::shared_ptr<ModelPar>,
                ParametrizationProxy>::const_iterator it =
           parametrizations.begin();
       it != parametrizations.end(); it++) {
    if (it->second.hasParametrization()) {
      int temp_error_code = it->second.getParametrization()->check();
      if (0 == temp_error_code) {
        it->second.getParametrization()->parametrize();
      }

      if (0 == error_code)
        error_code = temp_error_code;
    } else { // if its a parametrization model
      // update domain of that parametrization model and all things belonging to
      // it
      it->second.getParametrizationModel()->getModel()->init();
    }
  }
  return error_code;
}

bool ModelParameterHandler::hasParametrizationModels() const {
  for (std::map<const std::shared_ptr<ModelPar>,
                ParametrizationProxy>::const_iterator it =
           parametrizations.begin();
       it != parametrizations.end(); it++) {
    if (it->second.hasParametrizationModel()) {
      if (it->second.getParametrizationModel()
              ->getModelPar()
              ->isParameterFixed())
        return true;
      ;
    }
  }
  return false;
}

int ModelParameterHandler::checkParameters() {
  // check that all model parameters are set
  int error_code = model_par_set.checkParameters();
  return error_code;
}

void ModelParameterHandler::reinitModelParametrizations() {
  // just call the init function of all parametrizations
  for (std::map<const std::shared_ptr<ModelPar>,
                ParametrizationProxy>::const_iterator it =
           parametrizations.begin();
       it != parametrizations.end(); it++) {
    if (it->second.hasParametrization()) {
      it->second.getParametrization()->init();
    }
  }
}

void ModelParameterHandler::registerUpdaters() {

  // ok we need only to register parametrizations since parametrization models
  // are automatically called on every evaluation...

  // clear current updaters
  // updating_model_parametrizations.clear();

  // now register new updaters
  // loop over the model par set
  /* for (std::map<std::string, std::shared_ptr<ModelPar>,
   stringpair_comp>::iterator it = model_par_map.begin(); it !=
   model_par_map.end(); it++) {
   // only free parameters may have changed an generate changes in others
   if (!it->second->isParameterFixed()) {
   // loop over the connections of this parameter
   for (std::map<const std::shared_ptr<ModelPar>,
   std::shared_ptr<Parametrization> >::iterator it2 =
   it->second->getParameterConnections().begin();
   it2 != it->second->getParameterConnections().end(); it2++) {
   // the last requirement for a updater on this parameter is
   // that the connected parameter has to be fixed
   if (it2->first->isParameterFixed()) {
   updating_model_parametrizations.insert(it2->second);
   // Usually a fixed parameter is locked so it is guaranteed that the
   // variable is not changed anymore. In this case the updater function
   // needs to the writing rights on the value, hence the lock has to be
   // opened.
   model_par_map[it2->first->getName()]->setLocked(false);
   }
   }
   }
   }*/
}

void ModelParameterHandler::registerParametrization(
    std::shared_ptr<ModelPar> model_par,
    std::shared_ptr<Parametrization> parametrization) {
  if (model_par_set.modelParameterExists(model_par)) {
    parametrizations[model_par].setParametrization(parametrization);
  } else {
    // throw an exception that user is trying to register a parametrization to a
    // model, which does not define the given model parameter
  }
}

void ModelParameterHandler::registerParametrizations(
    ModelParSet &model_par_set_,
    std::shared_ptr<Parametrization> parametrization) {
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::iterator it =
           model_par_set_.getModelParameterMap().begin();
       it != model_par_set_.getModelParameterMap().end(); it++) {
    registerParametrization(it->second, parametrization);
  }
}

void ModelParameterHandler::registerParametrizationModel(
    std::shared_ptr<ModelPar> model_par,
    std::shared_ptr<ParametrizationModel> parametrization_model) {
  if (model_par_set.modelParameterExists(model_par)) {
    parametrization_model->setModelPar(model_par);
    parametrizations[model_par].setParametrizationModel(parametrization_model);
    model_par_set.addModelParameters(parametrizations[model_par]
                                         .getParametrizationModel()
                                         ->getModel()
                                         ->getModelParameterSet());
    model_par->setValue(0.0);    // this makes the model par set
    model_par->setLocked(false); // because it will be updated on each
                                 // evaluation we have to unlock it
  } else {
    // throw an exception that user is trying to register a parametrization to a
    // model, which does not define the given model parameter
  }
}

ParametrizationProxy
ModelParameterHandler::getParametrizationProxyForModelParameter(
    std::string name_) {
  if (model_par_set.modelParameterExists(name_)) {
    if (parametrizations.find(model_par_set.getModelParameter(name_)) !=
        parametrizations.end()) {
      return parametrizations[model_par_set.getModelParameter(name_)];
    } else {
      std::cout << "ERROR: The requested model parameter " << name_
                << " does not have a parametrization!" << std::endl;
      ParametrizationProxy tempproxy;
      return tempproxy;
    }
  } else {
    std::cout << "ERROR: The requested model parameter " << name_
              << " does not exist!" << std::endl;
    ParametrizationProxy tempproxy;
    return tempproxy;
  }
}

void ModelParameterHandler::executeParametrizationModels(const mydouble *x) {
  for (std::map<const std::shared_ptr<ModelPar>, ParametrizationProxy>::iterator
           it = parametrizations.begin();
       it != parametrizations.end(); it++) {
    if (it->second.hasParametrizationModel()) {
      // std::cout
      //		<<
      // it->second.getParametrizationModel()->getModelPar()->getName()
      //		<< std::endl;
      // if the model parameter is freed do NOT call the parametrization model
      if (it->second.getParametrizationModel()
              ->getModelPar()
              ->isParameterFixed()) {
        // it->second.getParametrizationModel()->getModel()->updateDomain();
        // std::cout << x[0] << " " << x[1] << std::endl;
        it->second.getParametrizationModel()->parametrize(x);
        // std::cout
        //		<<
        // it->second.getParametrizationModel()->getModelPar()->getValue()
        //		<< std::endl;
      }
    }
  }
}

void ModelParameterHandler::updateModelParameters() {
  // loop over all registered updater parametrizations which
  // adjust the dependent parameters
  /*for (std::map<std::shared_ptr<ModelPar>, ParametrizationProxy>::iterator it
   = parametrizations.begin(); it != parametrizations.end(); it++) { if
   (it->second.hasParametrization()) {
   it->second.getParametrization()->parametrize();
   }
   }*/
}

void ModelParameterHandler::initModelParametersFromFitResult(
    const ModelFitResult &fit_result) {
  const std::set<ModelStructs::minimization_parameter> &fit_params =
      fit_result.getFitParameters();
  for (std::set<ModelStructs::minimization_parameter>::const_iterator it =
           fit_params.begin();
       it != fit_params.end(); it++) {
    if (getModelParameterSet().modelParameterExists(it->name)) {
      std::shared_ptr<ModelPar> model_par =
          getModelParameterSet().getModelParameter(it->name);
      bool was_fixed = false;
      if (model_par->isParameterFixed()) {
        was_fixed = true;
        model_par->setParameterFixed(false);
      }
      model_par->setValue(it->value);
      if (was_fixed)
        model_par->setParameterFixed(true);
    }
  }
}
