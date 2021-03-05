/*
 * std::shared_ptr<ModelMinimizer>.h
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#ifndef MODELMINIMIZER_H_
#define MODELMINIMIZER_H_

#include "ModelControlParameter.h"
#include "ModelFitResult.h"

#include <memory>

/**
 * Abstract class for constructing minimizer objects. Concrete implementations
 * (for example ROOTMinimizer using ROOT's Minuit Minimizer) need to derive
 * from this class and implement the #minimize() function that interfaces to
 * the minimizer.
 */
class ModelMinimizer {
protected:
  // control parameter used for the minimization
  std::shared_ptr<ModelControlParameter> control_parameter;

public:
  ModelMinimizer();
  virtual ~ModelMinimizer();

  std::shared_ptr<ModelControlParameter> getControlParameter() const;
  void setControlParameter(
      std::shared_ptr<ModelControlParameter> control_parameter_);

  virtual void increaseFunctionCallLimit();

  virtual int minimize() = 0;

  int doMinimization();

  virtual ModelFitResult createModelFitResult() const = 0;
};

#endif /* MODELMINIMIZER_H_ */
