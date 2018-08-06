/*
 * PndLmdDPMModelParametrization.h
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#ifndef PNDLMDDPMMODELPARAMETRIZATION_H_
#define PNDLMDDPMMODELPARAMETRIZATION_H_

#include "core/Parametrization.h"

class PndLmdDPMModelParametrization : public Parametrization {
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
	PndLmdDPMModelParametrization(ModelParSet &model_par_set_);
	virtual ~PndLmdDPMModelParametrization();

	void initParameters();

	void adjustStrictParameters();
	void adjustNonStrictParameters();

	void parametrize();
};

#endif /* PNDLMDDPMMODELPARAMETRIZATION_H_ */
