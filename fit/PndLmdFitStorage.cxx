/*
 * PndLmdFitStorage.cxx
 *
 *  Created on: Mar 26, 2014
 *      Author: steve
 */

#include "PndLmdFitStorage.h"

PndLmdFitStorage::PndLmdFitStorage() {
  // TODO Auto-generated constructor stub

}

PndLmdFitStorage::~PndLmdFitStorage() {
  // TODO Auto-generated destructor stub
}

const std::map<PndLmdFitOptions, std::vector<ModelFitResult> >& PndLmdFitStorage::getFitResults() const {
  return fit_results;
}

std::vector<ModelFitResult> PndLmdFitStorage::getFitResults(
    const PndLmdFitOptions &fit_options) const {
  std::vector<ModelFitResult> return_result;
  auto const& fit_result = fit_results.find(fit_options);
  if (fit_result != fit_results.end())
    return_result = fit_result->second;

  return return_result;
}

void PndLmdFitStorage::addFitResult(const PndLmdFitOptions &fit_options,
    const ModelFitResult &fit_result_) {
  fit_results[fit_options].push_back(fit_result_);
}
