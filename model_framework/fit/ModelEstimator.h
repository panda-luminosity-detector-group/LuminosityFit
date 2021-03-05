/*
 * ModelEstimator.h
 *
 *  Created on: Jun 5, 2013
 *      Author: steve
 */

#ifndef MODELESTIMATOR_H_
#define MODELESTIMATOR_H_

#include "core/ModelStructs.h"
#include "fit/EstimatorOptions.h"
#include "fit/ModelControlParameter.h"

#include <memory>

#include <map>
#include <vector>

class Data;
class Model;
class ModelPar;

// A cost functor that implements the residual r = 10 - x.
struct CostFunctor {
  std::shared_ptr<ModelControlParameter> est;
  CostFunctor(std::shared_ptr<ModelControlParameter> est_) : est(est_) {}
  bool operator()(double const *const *x, double *residual) const {
    unsigned int size(est->getParameterList().size());
    mydouble xtemp[size];
    for (unsigned int i = 0; i < size; ++i) {
      xtemp[i] = (mydouble)*x[i];
    }
    residual[0] = est->evaluate(xtemp);
    return true;
  }
};

class ModelEstimator : public ModelControlParameter {
private:
  unsigned int nthreads;
  mydouble last_estimator_value;
  std::vector<mydouble> previous_values;
  mydouble initial_estimator_value;
  bool allow_initial_normalization;

  // list of free parameters
  std::map<std::pair<std::string, std::string>, std::shared_ptr<ModelPar>,
           ModelStructs::stringpair_comp>
      free_parameters;

  void insertParameters();

  void updateFreeModelParameters(const mydouble *new_values);

  // data
  std::shared_ptr<Data> data;

  // chopped data
  std::vector<std::shared_ptr<Data>> chopped_data;

  void chopData();

protected:
  // model used for fitting
  std::shared_ptr<Model> fit_model;

  EstimatorOptions estimator_options;

public:
  ModelEstimator(bool allow_initial_normalization_);
  virtual ~ModelEstimator();

  void setNumberOfThreads(unsigned int number_of_threads);

  const std::shared_ptr<Model> getModel() const;
  void setModel(std::shared_ptr<Model> new_model);

  const std::shared_ptr<Data> getData() const;
  void setData(std::shared_ptr<Data> new_data);

  void setInitialEstimatorValue(mydouble initial_estimator_value_);
  std::vector<std::shared_ptr<ModelPar>> &getFreeParameterList();

  mydouble getLastEstimatorValue() const;

  mydouble evaluate(const mydouble *par);

  void applyEstimatorOptions(const EstimatorOptions &estimator_options_);

  /**
   * The estimator function (chi2, likelihood, etc)
   */
  virtual mydouble eval(std::shared_ptr<Data> data) = 0;
};

#endif /* MODELESTIMATOR_H_ */
