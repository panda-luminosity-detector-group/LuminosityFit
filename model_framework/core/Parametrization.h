/*
 * Parametrization.h
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#ifndef PARAMETRIZATION_H_
#define PARAMETRIZATION_H_

#include "ModelParSet.h"

#include <vector>

class ModelPar;

/**
 * Parametrizations take care of setting parameters to specific values, either
 * for value initialization, or by an actual parametrization. Similar to Models
 * ModelParametrizations can depend on the domain variables of the fit function (so
 * non constant functions). However unlike Models, ModelParametrizations require a
 * #ModelParSet to work on. All of the connections between parameters should be set
 * in the constructor of the individual parametrization and the parametrization
 * function has to be overwritten and implement the actual parametrization
 * behaviour.
 */
class Parametrization {
protected:
  /**
   * Usually parameterizations require other parameters which have to be defined
   * from outside by the users. However composite models may have partial
   * parametrizations (which parametrize a certain part of the model), which can
   * require the same superior parameters. For this reason only pointers to
   * ModelPar objects are saved. Once this occurs a relinking of the parameters
   * will guarantee a single instance of this common superior parameter.
   */
  std::vector<std::shared_ptr<ModelPar> > dependency_parameters;

  ModelParSet &model_par_set;

public:
  Parametrization(ModelParSet &model_par_set_);

  virtual ~Parametrization();

  void init();

  virtual void initParameters() =0;

  virtual void parametrize() =0;

  int check() const;
};

#endif /* PARAMETRIZATION_H_ */
