/*
 * Model.cxx
 *
 *  Created on: Jan 8, 2013
 *      Author: steve
 */

#include "Model.h"

//#include <iostream>

Model::Model(std::string name_, unsigned int dimension_)
    : model_par_handler(name_), submodel_list(), name(name_),
      dimension(dimension_) {}

Model::~Model() {
  // clear map of ModelPars
}

mydouble Model::multiply(std::shared_ptr<Model> m1, std::shared_ptr<Model> m2,
                         const mydouble *x) const {
  if (m1->evaluate(x) == 0.0 || m2->evaluate(x) == 0.0)
    return 0.0;
  return m1->evaluate(x) * m2->evaluate(x);
}

mydouble Model::add(std::shared_ptr<Model> m1, std::shared_ptr<Model> m2,
                    const mydouble *x) const {
  return m1->evaluate(x) + m2->evaluate(x);
}

void Model::addModelToList(std::shared_ptr<Model> model) {
  submodel_list.push_back(model);
  if (model_par_handler.getModelParameterSet().addModelParameters(
          model->getModelParameterSet())) {
    // in case some ModelPar's have been reassigned in the daughter model
    // the model and its parametrizations have to be re-initialized
    model->reinit();
  }
}

unsigned int Model::getDimension() const { return dimension; }

std::string Model::getName() const { return name; }

ModelParSet &Model::getModelParameterSet() {
  return model_par_handler.getModelParameterSet();
}

void Model::executeParametrizationModels(const mydouble *x) {
  // first call through all submodels of this model
  for (unsigned int i = 0; i < submodel_list.size(); i++) {
    /*	std::cout<<submodel_list[i]<<std::endl;
     std::cout<<submodel_list[i]->getName()<<std::endl;*/
    submodel_list[i]->executeParametrizationModels(x);
  }
  // loop over all parametrization models...
  model_par_handler.executeParametrizationModels(x);
  if (model_par_handler.hasParametrizationModels())
    updateDomain();
}

std::pair<mydouble, mydouble> Model::getUncertaincy(const mydouble *x) const {
  return std::make_pair(0.0, 0.0);
}

mydouble Model::evaluate(const mydouble *x) {
  // executeParametrizationModels(x);
  return eval(x);
}

void Model::reinit() {
  model_par_handler.reinitModelParametrizations();
  initModelParameters();
}

int Model::init() {
  // first go through all submodels of this model
  for (unsigned int i = 0; i < submodel_list.size(); i++) {
    submodel_list[i]->init();
  }
  // now check on this model if everything is set
  // check that all parametrizations have their requirements fulfilled
  // and call them if possible
  model_par_handler.checkParametrizations();

  // update domain since parameters are different now
  updateDomain();

  // then check if all parameters of the model have now been set
  // by user and parametrizations
  int error_code = model_par_handler.checkParameters();
  if (0 == error_code)
    model_par_handler.registerUpdaters();
  return error_code;
}

ModelParameterHandler &Model::getModelParameterHandler() {
  return model_par_handler;
}

void Model::updateDomainForModelWithName(const std::string &name) {
  if (name.compare(getName()) == 0) {
    updateDomain();
    return;
  }
  for (unsigned int i = 0; i < submodel_list.size(); i++) {
    submodel_list[i]->updateDomainForModelWithName(name);
  }
}

void Model::updateModel() {
  // call update functions for all submodels
  for (unsigned int i = 0; i < submodel_list.size(); i++) {
    submodel_list[i]->updateModel();
  }
  // update the model parameters that really appear in this model
  model_par_handler.updateModelParameters();
  // finally recalculate the domain of all models that construct this model
  updateDomain();
}

void Model::setParametersUnmodified() {
  // call update functions for all submodels
  for (unsigned int i = 0; i < submodel_list.size(); i++) {
    submodel_list[i]->setParametersUnmodified();
  }
  for (auto model_par :
       model_par_handler.getModelParameterSet().getModelParameterMap()) {
    model_par.second->setModified(false);
  }
}
