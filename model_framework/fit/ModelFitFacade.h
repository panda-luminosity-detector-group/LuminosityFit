/*
 * ModelFitFacade.h
 *
 *  Created on: Oct 7, 2013
 *      Author: steve
 */

#ifndef MODELFITFACADE_H_
#define MODELFITFACADE_H_

#include "ModelEstimator.h"
#include "ModelMinimizer.h"
#include "core/Model1D.h"

class ModelFitFacade {
private:
  std::shared_ptr<Data> data;
  std::shared_ptr<Model> model;

  std::shared_ptr<ModelEstimator> estimator;
  std::shared_ptr<ModelMinimizer> minimizer;

  EstimatorOptions estimator_options;

public:
  ModelFitFacade();
  virtual ~ModelFitFacade();

  std::shared_ptr<Data> getData() const;
  std::shared_ptr<ModelEstimator> getEstimator() const;
  const EstimatorOptions &getEstimatorOptions() const;
  std::shared_ptr<ModelMinimizer> getMinimizer() const;
  std::shared_ptr<Model> getModel() const;
  void setData(std::shared_ptr<Data> data_);
  void setEstimator(std::shared_ptr<ModelEstimator> estimator_);
  void setEstimatorOptions(const EstimatorOptions &est_opt_);
  void setMinimizer(std::shared_ptr<ModelMinimizer> minimizer_);
  void setModel(std::shared_ptr<Model> model_);

  Data scanEstimatorSpace(const std::vector<std::string> &variable_names);

  std::vector<mydouble>
  findGoodStartParameters(const std::vector<std::string> &variable_names,
                          const std::vector<double> &search_factors);

  ModelFitResult Fit();
};

#endif /* MODELFITFACADE_H_ */
