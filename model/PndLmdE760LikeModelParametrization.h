/*
 * PndLmdDPMModelParametrization.h
 *
 *  Created on: Oct 17, 2014
 *      Author: anastasia
 */

#ifndef PNDLMDE760LIKEMODELPARAMETRIZATION_H_
#define PNDLMDE760LIKEMODELPARAMETRIZATION_H_

#include "core/Parametrization.h"

class PndLmdE760LikeModelParametrization : public Parametrization {
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
	PndLmdE760LikeModelParametrization(ModelParSet &model_par_set_);
	virtual ~PndLmdE760LikeModelParametrization();

	void initParameters();

	void adjustStrictParameters();
	void adjustNonStrictParameters();

	void parametrize();
};

#endif /* PNDLMDE760MODELPARAMETRIZATION_H_ */
