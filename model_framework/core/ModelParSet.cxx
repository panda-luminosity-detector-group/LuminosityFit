/*
 * ModelParSet.cxx
 *
 *  Created on: Feb 9, 2013
 *      Author: steve
 */

#include "ModelParSet.h"

#include <iostream>

ModelParSet::ModelParSet(std::string model_name_)
    : model_name(model_name_), model_par_map() {}

ModelParSet::~ModelParSet() {
  
}

unsigned int ModelParSet::getNumberOfParameters() const {
  return model_par_map.size();
}

unsigned int ModelParSet::getNumberOfFreeParameters() const {
  unsigned int nfree = 0;
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           model_par_map.begin();
       it != model_par_map.end(); it++) {
    if (!it->second->isParameterFixed())
      nfree++;
  }
  return nfree;
}

void ModelParSet::printInfo() const {
  std::cout << "Set contains " << model_par_map.size() << " entries"
            << std::endl;
  int counter = 0;
  std::cout << "************************************************************"
            << std::endl;
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           model_par_map.begin();
       it != model_par_map.end(); it++) {
    std::cout << "------------------------------------------------------------"
              << std::endl;
    std::cout << "parameter " << counter << "  (" << it->first.first << ":"
              << it->first.second << ")" << std::endl;
    std::cout << "name: " << it->second->getName() << std::endl;
    std::cout << "value: " << it->second->getValue() << std::endl;
    std::cout << "is fixed: " << it->second->isParameterFixed() << std::endl;
    std::cout << "is set: " << it->second->isSet() << std::endl;
    std::cout << "------------------------------------------------------------"
              << std::endl;
    counter++;
  }
  std::cout << "************************************************************"
            << std::endl;
}

const std::shared_ptr<ModelPar>
ModelParSet::addModelParameter(std::string name_, double value_, bool fixed_) {
  if (modelParameterExists(name_)) {
    std::cout << "(" << model_name << ") WARNING: This model parameter "
              << name_ << " already exists. Returning existing value reference!"
              << std::endl;
  } else {
    model_par_map[std::make_pair(model_name, name_)] =
        std::shared_ptr<ModelPar>(new ModelPar(name_, value_, fixed_));
  }
  return model_par_map[std::make_pair(model_name, name_)];
}

void ModelParSet::addModelParameter(std::shared_ptr<ModelPar> model_par) {
  model_par_map[std::make_pair(model_name, model_par->getName())] = model_par;
}

int ModelParSet::setModelParameterValue(const std::string &name_,
                                        mydouble value_) {
  if (modelParameterExists(name_)) {
    model_par_map[std::make_pair(model_name, name_)]->setValue(value_);
    return 0;
  } else {
    // we did not find the parameter to be defined in this model, but if it is
    // superior/global then we have to check only for the name of the parameter
    for (std::map<std::pair<std::string, std::string>,
                  std::shared_ptr<ModelPar>,
                  ModelStructs::stringpair_comp>::const_iterator it =
             model_par_map.begin();
         it != model_par_map.end(); it++) {
      if (it->first.second.compare(name_) == 0) {
        it->second->setValue(value_);
        return 0;
      }
    }
    std::cout << "(" << model_name << ") ERROR: Parameter " << name_
              << " was not found in the parameter set!" << std::endl;
    return 1;
  }
}

void ModelParSet::reassignParameter(std::shared_ptr<ModelPar> model_par) {
  model_par_map.erase(std::make_pair(model_name, model_par->getName()));
  addModelParameter(model_par);
}

bool ModelParSet::modelParameterExists(
    const std::shared_ptr<ModelPar> &model_par) const {
  return modelParameterExists(model_par->getName());
}

bool ModelParSet::modelParameterExists(const std::string &name_) const {
  return modelParameterExists(std::make_pair(model_name, name_));
}

bool ModelParSet::modelParameterExists(
    const std::pair<std::string, std::string> &name_) const {
  if (model_par_map.find(name_) != model_par_map.end())
    return true;
  else {
    return false;
  }
}

int ModelParSet::addModelParameters(ModelParSet &daughter_model_par_set) {
  int num_pars_reassigned = 0;
  // loop over all parameters to be added
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           daughter_model_par_set.getModelParameterMap().begin();
       it != daughter_model_par_set.getModelParameterMap().end(); it++) {
    // superior/global parameters are special
    if (it->second->isSuperior()) {
      // loop over all parameters that are already in the set
      bool found = false;
      for (std::map<std::pair<std::string, std::string>,
                    std::shared_ptr<ModelPar>,
                    ModelStructs::stringpair_comp>::const_iterator model_it =
               model_par_map.begin();
           model_it != model_par_map.end(); model_it++) {
        // check if this element already exists in this map
        if (model_it->first.second.compare(it->first.second) == 0) {
          // check that also this variable is superior
          if (model_it->second->isSuperior()) {
            // take that element and reassign the one which is supposed to be
            // added
            daughter_model_par_set.reassignParameter(model_it->second);
            num_pars_reassigned++;
            found = true;
            break;
          } else {
            // display a warning if we have a variable with the same name but is
            // not superior
            std::cout << "(" << model_name
                      << ") WARNING: While trying to add a superior model "
                         "parameter, a "
                         "parameter with equal name was found, however this "
                         "parameter is not "
                         "global/superior!"
                      << std::endl;
          }
        }
      }
      if (!found) {
        model_par_map[it->first] = it->second;
      }
    } else { // if its not global just check if its unique and add it
      if (model_par_map.find(it->first) == model_par_map.end()) {
        model_par_map[it->first] = it->second;
      } else { // otherwise we have a problem...
        std::cout << "(" << model_name << ") ERROR: Entry " << it->first.first
                  << ":" << it->first.second
                  << " already exists in the current model parameter set! "
                     "Skipping this entry..."
                  << std::endl;
      }
    }
  }
  return num_pars_reassigned;
}

const mydouble &
ModelParSet::getModelParameterValue(const std::string &name_) const {
  return model_par_map.at(std::make_pair(model_name, name_))->getValue();
}

std::shared_ptr<ModelPar> ModelParSet::getModelParameter(
    const std::pair<std::string, std::string> &name_) {
  if (!modelParameterExists(name_)) {
    // ok we should actually throw an exception here
    // because we can only end up here if user asks from a composite model
    // for a parameter that does not exist... which is a mistake...
    std::cout << "ERROR: The requested parameter " << name_.first << ":"
              << name_.second
              << " does not exist! Return a new parameter of this type which "
                 "is unused!"
              << " As this call is for composite models please make sure that "
                 "the model "
              << "and parameter name are correct." << std::endl;
    addModelParameter(name_.second);
  }
  return model_par_map.at(name_);
}

std::shared_ptr<ModelPar>
ModelParSet::getModelParameter(const std::string &name_) {
  if (!modelParameterExists(name_)) {
    // we did not find the parameter to be defined in this model, but if it is
    // superior/global then we have to check only for the name of the parameter
    for (std::map<std::pair<std::string, std::string>,
                  std::shared_ptr<ModelPar>,
                  ModelStructs::stringpair_comp>::const_iterator it =
             model_par_map.begin();
         it != model_par_map.end(); it++) {
      if (it->first.second.compare(name_) == 0) {
        return it->second;
      }
    }
    addModelParameter(name_);
  }
  return getModelParameter(std::make_pair(model_name, name_));
}

int ModelParSet::checkParameters() const {
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           model_par_map.begin();
       it != model_par_map.end(); it++) {
    if (!it->second->isSet()) {
      return 1;
    }
  }
  return 0;
}

bool ModelParSet::checkSuperiorParameters() const {
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           model_par_map.begin();
       it != model_par_map.end(); it++) {
    if (it->second->isSuperior() && !it->second->isSet())
      return false;
  }
  return true;
}

std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
         ModelStructs::stringpair_comp>
ModelParSet::getFreeModelParameters() const {
  std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
           ModelStructs::stringpair_comp>
      free_parameters;
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           model_par_map.begin();
       it != model_par_map.end(); it++) {
    if (!it->second->isParameterFixed()) {
      free_parameters.insert(*it);
    }
  }
  return free_parameters;
}

void ModelParSet::freeModelParameter(const std::string &name_) {
  if (modelParameterExists(name_)) {
    model_par_map[std::make_pair(model_name, name_)]->setParameterFixed(false);
  } else {
    // we did not find the parameter to be defined in this model, but if it is
    // superior/global then we have to check only for the name of the parameter
    for (std::map<std::pair<std::string, std::string>,
                  std::shared_ptr<ModelPar>,
                  ModelStructs::stringpair_comp>::const_iterator it =
             model_par_map.begin();
         it != model_par_map.end(); it++) {
      if (it->first.second.compare(name_) == 0) {
        it->second->setParameterFixed(false);
        return;
      }
    }
    std::cout << "(" << model_name << ") ERROR: Parameter " << name_
              << " was not found in the parameter set!" << std::endl;
  }
}

void ModelParSet::freeModelParameter(
    const std::pair<std::string, std::string> &name_pair_) {
  if (model_par_map.find(name_pair_) != model_par_map.end()) {
    model_par_map[name_pair_]->setParameterFixed(false);
  } else {
    std::cout << "(" << model_name << ") WARNING: The requested parameter "
              << name_pair_.first << ":" << name_pair_.second
              << " does not exist!" << std::endl;
  }
}

void ModelParSet::freeAllModelParameters() {
  for (std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
                ModelStructs::stringpair_comp>::const_iterator it =
           model_par_map.begin();
       it != model_par_map.end(); it++) {
    if (!it->second->isSuperior()) {
      it->second->setParameterFixed(false);
    }
  }
}

std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
         ModelStructs::stringpair_comp> &
ModelParSet::getModelParameterMap() {
  return model_par_map;
}
