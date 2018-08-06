/*
 * ParametrizationProxy.h
 *
 *  Created on: Apr 4, 2013
 *      Author: steve
 */

#ifndef PARAMETRIZATIONPROXY_H_
#define PARAMETRIZATIONPROXY_H_

#include "Parametrization.h"
#include "ParametrizationModel.h"

enum parametrization_state {
  NONE, PARAMETRIZATION, PARAMETRIZATION_MODEL
};

class ParametrizationProxy {
private:
	parametrization_state state;

  std::shared_ptr<ParametrizationModel> parametrization_model;
  std::shared_ptr<Parametrization> parametrization;

public:
  ParametrizationProxy();
  virtual ~ParametrizationProxy();

  bool hasParametrization() const;
  bool hasParametrizationModel() const;

  void setParametrization(std::shared_ptr<Parametrization> parametrization_);
  void setParametrizationModel(std::shared_ptr<ParametrizationModel> parametrization_model_);

  const std::shared_ptr<Parametrization>& getParametrization() const;
  const std::shared_ptr<ParametrizationModel>& getParametrizationModel() const;
};

#endif /* PARAMETRIZATIONPROXY_H_ */
