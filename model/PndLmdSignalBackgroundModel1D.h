/*
 * PndLmdSignalBackgroundModel1D.h
 *
 *  Created on: Dec 19, 2014
 *      Author: steve
 */

#ifndef PNDLMDSIGNALBACKGROUNDMODEL1D_H_
#define PNDLMDSIGNALBACKGROUNDMODEL1D_H_

#include <core/Model1D.h>

class PndLmdSignalBackgroundModel1D: public Model1D {
  shared_ptr<Model1D> signal;
  shared_ptr<Model1D> background;

	shared_ptr<ModelPar> signal_fraction;
	shared_ptr<ModelPar> background_fraction;

public:
	PndLmdSignalBackgroundModel1D(std::string name_, shared_ptr<Model1D> signal_,
			shared_ptr<Model1D> background_);
	virtual ~PndLmdSignalBackgroundModel1D();

	void initModelParameters();

	mydouble eval(const mydouble *x) const;

	void updateDomain();
};

#endif /* PNDLMDSIGNALBACKGROUNDMODEL1D_H_ */
