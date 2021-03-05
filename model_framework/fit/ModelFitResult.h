/*
 * ModelFitResult.h
 *
 *  Created on: May 23, 2013
 *      Author: steve
 */

#ifndef MODELFITRESULT_H_
#define MODELFITRESULT_H_

#include "../core/ModelStructs.h"
// ../core is needed for stupid rootcint to find the include

#include <set>
#include <string>

class ModelFitResult {
private:
  int fit_status;
  unsigned int num_data_points;
  std::set<ModelStructs::minimization_parameter> fit_parameters;
  double final_estimator_value;

  unsigned int getNumberOfDataPoints() const;

public:
  ModelFitResult();
  ModelFitResult(const ModelFitResult &fit_result);
  virtual ~ModelFitResult();

  double getFinalEstimatorValue() const;
  unsigned int getNDF() const;
  int getFitStatus() const;

  void setFinalEstimatorValue(double final_estimator_value_);
  void setNumberOfDataPoints(unsigned int num_data_points_);
  void setFitStatus(int fit_status_);

  void addFitParameter(std::pair<std::string, std::string> name_, double value_,
                       double error_);
  const ModelStructs::minimization_parameter
  getFitParameter(std::string name_) const;
  const ModelStructs::minimization_parameter &
  getFitParameter(std::pair<std::string, std::string> name_) const;
  const std::set<ModelStructs::minimization_parameter> &
  getFitParameters() const;
};

#endif /* MODELFITRESULT_H_ */
