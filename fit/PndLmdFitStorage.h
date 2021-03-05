/*
 * PndLmdFitStorage.h
 *
 *  Created on: Mar 26, 2014
 *      Author: steve
 */

#ifndef PNDLMDFITSTORAGE_H_
#define PNDLMDFITSTORAGE_H_

#include "PndLmdFitOptions.h"
#include "PndLmdLumiFitResult.h"
#include "fit/ModelFitResult.h"

#include <map>
#include <vector>

class PndLmdFitStorage {
  std::map<PndLmdFitOptions, std::vector<ModelFitResult>> fit_results;

public:
  PndLmdFitStorage();
  virtual ~PndLmdFitStorage();

  const std::map<PndLmdFitOptions, std::vector<ModelFitResult>> &
  getFitResults() const;
  std::vector<ModelFitResult>
  getFitResults(const PndLmdFitOptions &fit_options) const;
  void addFitResult(const PndLmdFitOptions &fit_options,
                    const ModelFitResult &fit_result_);
};

#endif /* PNDLMDFITSTORAGE_H_ */
