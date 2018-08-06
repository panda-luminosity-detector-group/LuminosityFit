/*
 * PndLmdDPMModelParametrization.h
 *
 *  Created on: Oct 17, 2014
 *      Author: anastasia
 */

#ifndef PNDLMDE760MODELPARAMETRIZATION_H_
#define PNDLMDE760MODELPARAMETRIZATION_H_

#include "core/Parametrization.h"

class PndLmdE760ModelParametrization : public Parametrization {
private:
	std::shared_ptr<ModelPar> p_lab;
	std::shared_ptr<ModelPar> E_lab;
	std::shared_ptr<ModelPar> S;
	std::shared_ptr<ModelPar> pcm2;
	std::shared_ptr<ModelPar> gamma;
	std::shared_ptr<ModelPar> beta;
	std::shared_ptr<ModelPar> beta_lab_cms;

	std::shared_ptr<ModelPar> A1;
	std::shared_ptr<ModelPar> T1;
	std::shared_ptr<ModelPar> A2;
	std::shared_ptr<ModelPar> T2;
	std::shared_ptr<ModelPar> A3;
	std::shared_ptr<ModelPar> rho;
	std::shared_ptr<ModelPar> b;
	std::shared_ptr<ModelPar> sigma_tot;

public:
	PndLmdE760ModelParametrization(ModelParSet &model_par_set_);
	virtual ~PndLmdE760ModelParametrization();

	void initParameters();

	void adjustStrictParameters();
	void adjustNonStrictParameters();

	void parametrize();
};

#endif /* PNDLMDE760MODELPARAMETRIZATION_H_ */
