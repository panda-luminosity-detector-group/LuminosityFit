/*
 * ROOTMinimizer.h
 *
 *  Created on: Jan 21, 2013
 *      Author: steve
 */

#ifndef ROOTMINIMIZER_H_
#define ROOTMINIMIZER_H_

#include "fit/ModelMinimizer.h"
#include "fit/ModelFitResult.h"

#include "Math/Minimizer.h"

class ROOTMinimizer: public ModelMinimizer {
private:
	ROOT::Math::Minimizer* min;

	int minimize();

  double root_func_wrapper(const double *x);

public:
	ROOTMinimizer(int type=0);
	virtual ~ROOTMinimizer();

	void increaseFunctionCallLimit();

	const ROOT::Math::Minimizer* getROOTMinimizer() const;

	virtual ModelFitResult createModelFitResult() const;
};

#endif /* ROOTMINIMIZER_H_ */
