#ifndef PNDLMDSMEARINGCONVOLUTIONMODEL2D_H_
#define PNDLMDSMEARINGCONVOLUTIONMODEL2D_H_

#include "core/Model2D.h"
#include "PndLmdSmearingModel2D.h"

class PndLmdSmearingConvolutionModel2D: public Model2D {
	shared_ptr<Model2D> unsmeared_model;
	shared_ptr<PndLmdSmearingModel2D> smearing_model;

public:
	PndLmdSmearingConvolutionModel2D(std::string name_,
			shared_ptr<Model2D> unsmeared_model_,
			shared_ptr<PndLmdSmearingModel2D> smearing_model_);
	virtual ~PndLmdSmearingConvolutionModel2D();

	void initModelParameters();

	void injectModelParameter(shared_ptr<ModelPar> model_param);

	mydouble eval(const double *x) const;

	void updateDomain();
};

#endif /* PNDLMDSMEARINGCONVOLUTIONMODEL2D_H_ */
