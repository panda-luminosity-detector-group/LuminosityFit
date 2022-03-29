/*
 * ModelFitFacade.cxx
 *
 *  Created on: Oct 7, 2013
 *      Author: steve
 */

#include "ModelFitFacade.h"
#include "fit/data/Data.h"
#include <algorithm>
#include <iostream>

using std::cout;
using std::endl;

ModelFitFacade::ModelFitFacade()
    : data(), model(), estimator(), minimizer(), estimator_options() {}

ModelFitFacade::~ModelFitFacade() {
  
}

const EstimatorOptions &ModelFitFacade::getEstimatorOptions() const {
  return estimator_options;
}

void ModelFitFacade::setEstimatorOptions(const EstimatorOptions &est_opt_) {
  estimator_options = est_opt_;
}

std::shared_ptr<Data> ModelFitFacade::getData() const { return data; }

std::shared_ptr<ModelEstimator> ModelFitFacade::getEstimator() const {
  return estimator;
}

std::shared_ptr<ModelMinimizer> ModelFitFacade::getMinimizer() const {
  return minimizer;
}

std::shared_ptr<Model> ModelFitFacade::getModel() const { return model; }

void ModelFitFacade::setData(std::shared_ptr<Data> data_) { data = data_; }

void ModelFitFacade::setEstimator(std::shared_ptr<ModelEstimator> estimator_) {
  estimator = estimator_;
}

void ModelFitFacade::setMinimizer(std::shared_ptr<ModelMinimizer> minimizer_) {
  minimizer = minimizer_;
}

void ModelFitFacade::setModel(std::shared_ptr<Model> model_) { model = model_; }

Data ModelFitFacade::scanEstimatorSpace(
    const std::vector<std::string> &variable_names) {
  Data scan_data(2);

  // check that estimator is set
  if (!estimator) {
    std::cout << "Estimator not set...\n";
    return scan_data;
  }

  // check if data and model have the correct dimensions
  if (model->getDimension() != data->getDimension()) {
    std::cout << "The model has a dimension of " << model->getDimension()
              << ", which does not match the data dimension of "
              << data->getDimension() << "!" << std::endl;
    return scan_data;
  }

  if (variable_names.size() > 2) {
    std::cout << "only two variable scan supported...\n";
    return scan_data;
  }

  // set model
  estimator->setModel(model);
  // set data
  estimator->setData(data);
  // apply estimator options
  std::cout << "applying estimator options (in case of integral scaling this "
               "can mean integrals are being computed!)..."
            << std::endl;
  estimator->applyEstimatorOptions(estimator_options);

  std::cout << "Now performing actual scan!!!!\n";
  // find the correct parameters first
  auto free_params = model->getModelParameterSet().getFreeModelParameters();
  mydouble params[free_params.size()];
  unsigned int index_1(0);
  unsigned int index_2(0);
  unsigned int counter(0);
  for (auto param : free_params) {
    if (param.first.second.compare(variable_names[0]) == 0)
      index_1 = counter;
    else if (param.first.second.compare(variable_names[1]) == 0)
      index_2 = counter;
    params[counter] = param.second->getValue();
    ++counter;
  }
  int num_bins = 40;
  mydouble scanwidth(0.5L);
  mydouble x_center = params[index_1];
  mydouble y_center = params[index_2];
  mydouble stepsize_x = x_center * scanwidth / num_bins;
  mydouble stepsize_y = y_center * scanwidth / num_bins;

  DataPointProxy dpp;
  for (int ix = -num_bins; ix < num_bins + 1; ++ix) {
    for (int iy = -num_bins; iy < num_bins + 1; ++iy) {
      std::shared_ptr<DataStructs::binned_data_point> bdp(
          new DataStructs::binned_data_point);
      bdp->bin_widths[0] = stepsize_x;
      bdp->bin_widths[1] = stepsize_y;
      dpp.setBinnedDataPoint(bdp);
      params[index_1] = x_center + stepsize_x * ix;
      params[index_2] = y_center + stepsize_y * iy;
      bdp->bin_center_value[0] = params[index_1];
      bdp->bin_center_value[1] = params[index_2];
      bdp->z = estimator->evaluate(params);
      scan_data.insertData(dpp);
      std::cout << "adding: " << bdp->bin_center_value[0] << " : "
                << bdp->bin_center_value[1] << " = " << bdp->z << std::endl;
    }
  }

  return scan_data;
}

std::vector<mydouble> ModelFitFacade::findGoodStartParameters(
    const std::vector<std::string> &variable_names,
    const std::vector<double> &search_factors) {
  std::vector<mydouble> best_parameters(variable_names.size());
  // check that estimator is set
  if (!estimator) {
    throw std::runtime_error(
        "ModelFitFacade::findGoodStartParameters: Estimator not set...");
  }

  // check if data and model have the correct dimensions
  if (model->getDimension() != data->getDimension()) {
    std::cout << "The model has a dimension of " << model->getDimension()
              << ", which does not match the data dimension of "
              << data->getDimension() << "!" << std::endl;
    throw std::runtime_error(
        "ModelFitFacade::findGoodStartParameters: dimension missmatch!");
  }

  // set model
  estimator->setModel(model);
  // set data
  estimator->setData(data);
  // apply estimator options
  std::cout << "applying estimator options (in case of integral scaling this "
               "can mean integrals are being computed!)..."
            << std::endl;
  estimator->applyEstimatorOptions(estimator_options);

  std::cout << "Finding good start parameters for parameters!\n";

  // find the correct parameters first
  auto free_params = model->getModelParameterSet().getFreeModelParameters();
  std::vector<mydouble> params(free_params.size());
  std::vector<unsigned int> indices(variable_names.size());

  unsigned int counter(0);
  unsigned int num_counter(0);
  for (auto param : free_params) {
    for (unsigned int i = 0; i < variable_names.size(); ++i) {
      if (param.first.second.compare(variable_names[i]) == 0) {
        indices[i] = counter;
        ++num_counter;
        break;
      }
    }
    params[counter] = param.second->getValue();
    ++counter;
  }
  if (num_counter != variable_names.size())
    throw std::runtime_error(
        "ModelFitFacade::findGoodStartParameters: requesting scan for "
        "parameters that are not free parameters of the fit!");

  std::vector<std::vector<mydouble>> scan_grid;
  std::vector<std::vector<mydouble>> temp_set;
  for (unsigned int i = 0; i < indices.size(); ++i) {
    if (scan_grid.size() == 0) {
      std::vector<mydouble> temp_params(params);
      for (double search_factor : search_factors) {
        temp_params[indices[i]] = params[indices[i]] / search_factor;
        temp_set.push_back(temp_params);
        temp_params[indices[i]] = params[indices[i]] * search_factor;
        temp_set.push_back(temp_params);
      }
      temp_params[indices[i]] = params[indices[i]];
      temp_set.push_back(temp_params);

    } else {
      for (auto const &set : scan_grid) {
        std::vector<mydouble> temp_params(set);
        for (double search_factor : search_factors) {
          temp_params[indices[i]] = params[indices[i]] / search_factor;
          temp_set.push_back(temp_params);
          temp_params[indices[i]] = params[indices[i]] * search_factor;
          temp_set.push_back(temp_params);
        }
        temp_params[indices[i]] = params[indices[i]];
        temp_set.push_back(temp_params);
      }
    }
    scan_grid = temp_set;
    temp_set.clear();
  }

  std::vector<mydouble> estimator_values;

  std::cout << "scanning " << scan_grid.size() << " points!\n";
  for (auto const &point : scan_grid) {
    mydouble temp(estimator->evaluate(&point[0]));
    estimator_values.push_back(temp);
  }

  // normalize to mean and find best
  mydouble mean(0.0);
  for (mydouble a : estimator_values)
    mean += a;
  mean /= estimator_values.size();
  for (mydouble &a : estimator_values)
    a = a - mean;
  unsigned int best_index =
      std::min_element(estimator_values.begin(), estimator_values.end()) -
      estimator_values.begin();

  for (unsigned int i = 0; i < indices.size(); ++i)
    best_parameters[i] = scan_grid[best_index][indices[i]];

  std::cout << "best parameters are: \n";
  for (unsigned int i = 0; i < indices.size(); ++i) {
    std::cout << variable_names[i] << ": " << best_parameters[i] << std::endl;
  }
  return best_parameters;
}

ModelFitResult ModelFitFacade::Fit() {
  ModelFitResult fit_result_dummy;

  // check that estimator is set
  if (!estimator) {
    fit_result_dummy.setFitStatus(-1);
    return fit_result_dummy;
  }

  // check if data and model have the correct dimensions
  if (model->getDimension() != data->getDimension()) {
    std::cout << "The model has a dimension of " << model->getDimension()
              << ", which does not match the data dimension of "
              << data->getDimension() << "!" << std::endl;
    return fit_result_dummy;
  }

  // set model
  estimator->setModel(model);
  // set data
  estimator->setData(data);
  // apply estimator options
  std::cout << "applying estimator options (in case of integral scaling this "
               "can mean integrals are being computed!)..."
            << std::endl;
  estimator->applyEstimatorOptions(estimator_options);

  // check that minimizer exists
  if (!minimizer) {
    fit_result_dummy.setFitStatus(-2);
    return fit_result_dummy;
  }

  minimizer->setControlParameter(estimator);

  auto const &free_params =
      model->getModelParameterSet().getFreeModelParameters();
  std::cout << free_params.size() << " free parameters in fit\n";
  std::vector<mydouble> pars;
  for (auto const &param : free_params) {
    pars.push_back(param.second->getValue());
  }

  int fit_status(-1);
  // this try loop is done to keep normalizing the estimator space to get better
  // numerical stability
  for (unsigned int trys = 0; trys < 3; ++trys) {
    estimator->setInitialEstimatorValue(estimator->evaluate(&pars[0]));

    // call minimization procedure
    fit_status = minimizer->doMinimization();

    std::cout << "try: " << trys << " finished with fit status: " << fit_status
              << std::endl;
    // if fit was successful this try was successful
    if (fit_status == 0)
      break;
    else {
      // if not successful use last parameters as new start value and repeat!
      unsigned int counter(0);
      auto &parameters = minimizer->getControlParameter()->getParameterList();
      for (auto const &param : free_params) {
        parameters[counter].value = param.second->getValue();
        pars[counter] = param.second->getValue();
        ++counter;
      }
      // reset initial estimator value
      estimator->setInitialEstimatorValue(0.0);
    }
    // minimizer->increaseFunctionCallLimit();
  }
  if (fit_status) {
    cout << "ERROR: Problem while performing fit. Using last parameters!"
         << endl;
  }

  ModelFitResult fit_result = minimizer->createModelFitResult();
  fit_result.setFitStatus(fit_status);

  fit_result.setFinalEstimatorValue(estimator->getLastEstimatorValue());
  fit_result.setNumberOfDataPoints(
      estimator->getData()->getNumberOfUsedDataPoints());
  return fit_result;
}
