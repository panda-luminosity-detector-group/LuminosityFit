/*
 * ModelPar.h
 *
 *  Created on: Jan 12, 2013
 *      Author: steve
 */

#ifndef MODELPAR_H_
#define MODELPAR_H_

#include <string>
#include <set>

#include <memory>
#include "ProjectWideSettings.h"

class ModelPar {
private:
  std::string name;
  mydouble value;
  mydouble lower_bound;
  mydouble upper_bound;
  bool fixed, superior, set, locked, bounded;
  bool modified;

  /**
   * This stores the connection to other ModelPar's. Once this ModelPar is
   * freed, the parametrizations (if they exist) of the connected parameters
   * are called at each iteration step. Note that a parameter can also have a
   * connection to itself to update parameters that are used directly within a
   * model.
   */
  std::set<std::shared_ptr<ModelPar> > connections;

public:
  ModelPar();
  ModelPar(std::string name_, mydouble value_, bool fixed_);

  const std::string& getName() const;

  void setLocked(bool locked_);

  bool isParameterFixed() const;
  void setParameterFixed(bool fixed_);

  void setParameterBounds(mydouble lower_bound_, mydouble upper_bound_);

  /**
   * Sets the value of this model parameter. Note that only by using this function
   * the set flag will be switched to true, which is a requirement for the fit.
   * Hence this method has to be used by the model parametrizations.
   */
  void setValue(mydouble value_);
  const mydouble& getValue() const;

  bool isSet() const;
  bool isConnected() const;
  bool isSuperior() const;
  bool isModified() const;

  void setSuperior(bool superior_);
  void setModified(bool modified_);
  /**
   * This function adds a dependence of this model parameter on the specified
   * parameter. Once the dependent parameter changed this model parameter will
   * be updated via the specified parametrization in the corresponding handler
   * instance.
   */
  void setConnectionTo(const std::shared_ptr<ModelPar> &model_par);

  std::set<std::shared_ptr<ModelPar> >& getParameterConnections();

};

#endif /* MODELPAR_H_ */
