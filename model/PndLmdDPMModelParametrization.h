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
	shared_ptr<ModelPar> p_lab;
	shared_ptr<ModelPar> E_lab;
	shared_ptr<ModelPar> S;
	shared_ptr<ModelPar> pcm2;
	shared_ptr<ModelPar> gamma;
	shared_ptr<ModelPar> beta;
	shared_ptr<ModelPar> beta_lab_cms;

	shared_ptr<ModelPar> A1;
	shared_ptr<ModelPar> T1;
	shared_ptr<ModelPar> A2;
	shared_ptr<ModelPar> T2;
	shared_ptr<ModelPar> A3;
	shared_ptr<ModelPar> rho;
	shared_ptr<ModelPar> b;
	shared_ptr<ModelPar> sigma_tot;

public:
	PndLmdDPMModelParametrization(ModelParSet &model_par_set_);
	virtual ~PndLmdDPMModelParametrization();

	void initParameters();

	void adjustStrictParameters();
	void adjustNonStrictParameters();

	void parametrize();
};

#endif /* PNDLMDDPMMODELPARAMETRIZATION_H_ */
