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
  std::shared_ptr<Model1D> signal;
  std::shared_ptr<Model1D> background;

	std::shared_ptr<ModelPar> signal_fraction;
	std::shared_ptr<ModelPar> background_fraction;

public:
	PndLmdSignalBackgroundModel1D(std::string name_, std::shared_ptr<Model1D> signal_,
			std::shared_ptr<Model1D> background_);
	virtual ~PndLmdSignalBackgroundModel1D();

	void initModelParameters();

	mydouble eval(const mydouble *x) const;

	void updateDomain();
};

#endif /* PNDLMDSIGNALBACKGROUNDMODEL1D_H_ */
