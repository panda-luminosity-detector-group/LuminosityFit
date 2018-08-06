#ifndef SMEARINGCONVOLUTIONMODEL2D_H_
#define SMEARINGCONVOLUTIONMODEL2D_H_

#include "core/Model2D.h"

class SmearingConvolutionModel2D: public Model2D {
private:
	unsigned int divisions_var1;
	unsigned int divisions_var2;

	std::shared_ptr<Model2D> first, second;

public:
	SmearingConvolutionModel2D(std::string name_, std::shared_ptr<Model2D> first_,
			std::shared_ptr<Model2D> second_);
	virtual ~SmearingConvolutionModel2D();

	void initModelParameters();

	void calculateLookupTable();

	mydouble eval(const mydouble *x) const;

	void updateDomain();
};

#endif /* SMEARINGCONVOLUTIONMODEL2D_H_ */
