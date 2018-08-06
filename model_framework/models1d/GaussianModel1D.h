/*
 * GaussianModel1D.h
 *
 *  Created on: Dec 17, 2012
 *      Author: steve
 */

#ifndef GAUSSIANMODEL1D_H_
#define GAUSSIANMODEL1D_H_

#include "core/Model1D.h"

class GaussianModel1D: public Model1D {
private:
  mydouble num_sigmas;
	std::shared_ptr<ModelPar> gauss_sigma;
	std::shared_ptr<ModelPar> gauss_mean;
	std::shared_ptr<ModelPar> gauss_amplitude;

public:
	/**
	 * The constructor for creating a normalized gaussian model in 1D
	 * (normal distribution).
	 * @params name_ will be set as the name of this model. Make sure this will be unique!
	 * */
	GaussianModel1D(std::string name_);

	virtual ~GaussianModel1D();

	void initModelParameters();

	/**
	 * normalized detector response function for 1D fits. Here: 1D-gaussian
	 * @params x pointer to array containing theta value
	 * @returns value of the response function at the specified theta value with
	 * the given theta sigma
	 */
	mydouble eval(const mydouble *x) const;

	void updateDomain();
};

#endif /* GAUSSIANMODEL1D_H_ */
