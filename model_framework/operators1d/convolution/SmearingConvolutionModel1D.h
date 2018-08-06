/*
 * SmearingConvolutionModel1D.h
 *
 *  Created on: Jul 9, 2013
 *      Author: steve
 */

#ifndef SMEARINGCONVOLUTIONMODEL1D_H_
#define SMEARINGCONVOLUTIONMODEL1D_H_

#include "core/Model1D.h"

class SmearingConvolutionModel1D: public Model1D {
private:
	unsigned int divisions;

	std::shared_ptr<Model1D> first, second;
public:
	SmearingConvolutionModel1D(std::string name_, std::shared_ptr<Model1D> first_,
			std::shared_ptr<Model1D> second_);
	virtual ~SmearingConvolutionModel1D();

	void initModelParameters();

	mydouble eval(const mydouble *x) const;

	void updateDomain();
};

#endif /* SMEARINGCONVOLUTIONMODEL1D_H_ */
