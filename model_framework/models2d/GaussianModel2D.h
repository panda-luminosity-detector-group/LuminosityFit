/*
 * GaussianModel2D.h
 *
 *  Created on: Jan 16, 2013
 *      Author: steve
 */

#ifndef GAUSSIANMODEL2D_H_
#define GAUSSIANMODEL2D_H_

#include "core/Model2D.h"

class GaussianModel2D: public Model2D {
private:
	mydouble num_sigmas;
	std::shared_ptr<ModelPar> gauss_sigma_var1;
	std::shared_ptr<ModelPar> gauss_sigma_var2;
	std::shared_ptr<ModelPar> gauss_mean_var1;
	std::shared_ptr<ModelPar> gauss_mean_var2;
	std::shared_ptr<ModelPar> gauss_rho;
	std::shared_ptr<ModelPar> gauss_amplitude;

public:
	/**
	 * The constructor for creating a normalized gaussian model in 2D
	 * (normal distribution).
	 * @params name_ will be set as the name of this model. Make sure this will be unique!
	 */
	GaussianModel2D(std::string name_, mydouble num_sigmas_ = 5.0);

	virtual ~GaussianModel2D();

	void initModelParameters();

	/**
	 * normalized detector response function for 2D fits. Here: 2D-gaussian
	 * @params x pointer to array containing theta value
	 * @returns value of the response function at the specified theta value with
	 * the given theta sigma
	 */
	mydouble eval(const mydouble *x) const;

	void updateDomain();
};

#endif /* GAUSSIANMODEL2D_H_ */
