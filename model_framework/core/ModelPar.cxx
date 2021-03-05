/*
 * ModelPar.cxx
 *
 *  Created on: Jan 12, 2013
 *      Author: steve
 */

#include "ModelPar.h"

ModelPar::ModelPar()
    : name(""), value(0.0), lower_bound(0.0), upper_bound(0.0), fixed(true),
      superior(false), set(false), locked(false), bounded(false),
      connections() {}

ModelPar::ModelPar(std::string name_, mydouble value_, bool fixed_)
    : name(name_), value(value_), lower_bound(0.0), upper_bound(0.0),
      fixed(fixed_), superior(false), set(false), locked(true),
      // variable is immediately locked which sounds strange
      // but it can be set once because it is not set yet
      // the lock should not be fiddled with and is automatically
      // opened within the registerUpdater method of #ModelParameterHandler
      bounded(false), modified(true), connections() {}

const std::string &ModelPar::getName() const { return name; }

void ModelPar::setLocked(bool locked_) { locked = locked_; }

bool ModelPar::isParameterFixed() const { return fixed; }

void ModelPar::setParameterFixed(bool fixed_) { fixed = fixed_; }

void ModelPar::setParameterBounds(mydouble lower_bound_,
                                  mydouble upper_bound_) {
  lower_bound = lower_bound_;
  upper_bound = upper_bound_;
  bounded = true;
}

void ModelPar::setValue(mydouble value_) {
  // there are some requirements to allow the setting of the parameter
  // if it wasn't set at all before...
  if (!set) {
    value = value_;
    set = true;
  } else {
    // usually if its set you don't want it to change again
    // there is an exception:
    // is fixed and is connected to another parameter
    // and that connection partner has changed
    if (!locked || !fixed) {
      value = value_;
      modified = true;
    }
  }
}

const mydouble &ModelPar::getValue() const { return value; }

bool ModelPar::isSet() const { return set; }
bool ModelPar::isConnected() const {
  if (connections.size() > 0)
    return true;
  else
    return false;
}
bool ModelPar::isSuperior() const { return superior; }
bool ModelPar::isModified() const { return modified; }

void ModelPar::setSuperior(bool superior_) { superior = superior_; }

void ModelPar::setModified(bool modified_) { modified = modified_; }

void ModelPar::setConnectionTo(const std::shared_ptr<ModelPar> &model_par) {
  connections.insert(model_par);
}

std::set<std::shared_ptr<ModelPar>> &ModelPar::getParameterConnections() {
  return connections;
}
