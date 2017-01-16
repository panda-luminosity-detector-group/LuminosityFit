/*
 * AsymmetricGaussianModel1D.h
 *
 *  Created on: Dec 17, 2012
 *      Author: steve
 */

#ifndef ASYMMETRICGAUSSIANMODEL1D_H_
#define ASYMMETRICGAUSSIANMODEL1D_H_

#include "core/Model1D.h"

class AsymmetricGaussianModel1D: public Model1D {
private:
	double num_sigmas;
	shared_ptr<ModelPar> asymm_gauss_sigma_left;
	shared_ptr<ModelPar> asymm_gauss_sigma_right;
	shared_ptr<ModelPar> asymm_gauss_mean;
	shared_ptr<ModelPar> asymm_gauss_amplitude;

public:
	/**
	 * The constructor for creating an asymmetric gaussian model in 1D
	 * @params name_ will be set as the name of this model. Make sure this will be unique!
	 */
	AsymmetricGaussianModel1D(std::string name_);

	virtual ~AsymmetricGaussianModel1D();

	void initModelParameters();

	mydouble eval(const double *x) const;

	void updateDomain();
};

#endif /* ASYMMETRICGAUSSIANMODEL1D_H_ */
