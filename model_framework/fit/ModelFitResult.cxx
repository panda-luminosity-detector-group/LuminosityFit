/*
 * ModelFitResult.cxx
 *
 *  Created on: May 23, 2013
 *      Author: steve
 */

#include "ModelFitResult.h"

#include <iostream>

ModelFitResult::ModelFitResult()
    : fit_status(-1), num_data_points(0), fit_parameters(),
      final_estimator_value(0.0) {}

ModelFitResult::ModelFitResult(const ModelFitResult &fit_result) {
  setFitStatus(fit_result.getFitStatus());
  setFinalEstimatorValue(fit_result.getFinalEstimatorValue());
  setNumberOfDataPoints(fit_result.getNumberOfDataPoints());
  for (std::set<ModelStructs::minimization_parameter>::const_iterator
           fit_param = fit_result.getFitParameters().begin();
       fit_param != fit_result.getFitParameters().end(); fit_param++) {
    addFitParameter(fit_param->name, fit_param->value, fit_param->error);
  }
}

ModelFitResult::~ModelFitResult() {
  
}

unsigned int ModelFitResult::getNumberOfDataPoints() const {
  return num_data_points;
}

double ModelFitResult::getFinalEstimatorValue() const {
  return final_estimator_value;
}

unsigned int ModelFitResult::getNDF() const {
  return num_data_points - fit_parameters.size();
}

void ModelFitResult::setFinalEstimatorValue(double final_estimator_value_) {
  this->final_estimator_value = final_estimator_value_;
}

void ModelFitResult::setNumberOfDataPoints(unsigned int num_data_points_) {
  this->num_data_points = num_data_points_;
}

void ModelFitResult::addFitParameter(std::pair<std::string, std::string> name_,
                                     double value_, double error_) {
  ModelStructs::minimization_parameter fp(name_, value_, error_);
  fit_parameters.insert(fp);
}

const ModelStructs::minimization_parameter
ModelFitResult::getFitParameter(std::string name_) const {
  for (std::set<ModelStructs::minimization_parameter>::const_iterator
           min_param = fit_parameters.begin();
       min_param != fit_parameters.end(); min_param++) {
    if (min_param->name.second.compare(name_) == 0) {
      return *min_param;
    }
  }
  std::cout << "ERROR: requesting value of superior parameter " << name_
            << " which is unknown!" << std::endl;
  throw 1;
}

const ModelStructs::minimization_parameter &ModelFitResult::getFitParameter(
    std::pair<std::string, std::string> name_) const {
  ModelStructs::minimization_parameter fp(name_);
  if (fit_parameters.find(fp) == fit_parameters.end()) {
    std::cout << "ERROR: requesting value of parameter " << name_.first << ":"
              << name_.second << " which is unknown!" << std::endl;
  }
  return *fit_parameters.find(fp);
}

const std::set<ModelStructs::minimization_parameter> &
ModelFitResult::getFitParameters() const {
  return fit_parameters;
}

int ModelFitResult::getFitStatus() const { return fit_status; }

void ModelFitResult::setFitStatus(int fit_status_) { fit_status = fit_status_; }
