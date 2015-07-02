/*
 * PndLmdFitStorage.h
 *
 *  Created on: Mar 26, 2014
 *      Author: steve
 */

#ifndef PNDLMDFITSTORAGE_H_
#define PNDLMDFITSTORAGE_H_

#include "PndLmdLumiFitResult.h"
#include "PndLmdFitOptions.h"
#include "fit/ModelFitResult.h"

#include <map>

using std::map;

class PndLmdFitStorage {
	map<PndLmdFitOptions, ModelFitResult> fit_results;

public:
	PndLmdFitStorage();
	virtual ~PndLmdFitStorage();

	const map<PndLmdFitOptions, ModelFitResult>& getFitResults() const;
	ModelFitResult getFitResult(const PndLmdFitOptions &fit_options) const;
	void addFitResult(const PndLmdFitOptions &fit_options, const ModelFitResult &fit_result_);
};

#endif /* PNDLMDFITSTORAGE_H_ */
