#ifndef PNDLMDDIFFERENTIALSMEARINGCONVOLUTIONMODEL2D_H_
#define PNDLMDDIFFERENTIALSMEARINGCONVOLUTIONMODEL2D_H_

#include "core/Model2D.h"
#include "PndLmdDivergenceSmearingModel2D.h"

class PndLmdDifferentialSmearingConvolutionModel2D: public Model2D {
	struct binrange {
		unsigned x_bin_low;
		unsigned x_bin_high;
		unsigned y_bin_low;
		unsigned y_bin_high;
	};

	shared_ptr<Model2D> unsmeared_model;
	shared_ptr<PndLmdDivergenceSmearingModel2D> smearing_model;

	LumiFit::LmdDimension data_dim_x;
	LumiFit::LmdDimension data_dim_y;

	double **model_grid;

	unsigned int nthreads;
	std::vector<binrange> list_of_bin_ranges;

	std::pair<double, double> binsizes;
	double area_xy;

	void generateModelGrid2D();

	void generateModelGrid2D(const binrange &br);

public:
	PndLmdDifferentialSmearingConvolutionModel2D(std::string name_,
			shared_ptr<Model2D> unsmeared_model_,
			shared_ptr<PndLmdDivergenceSmearingModel2D> smearing_model_,
			const LumiFit::LmdDimension& data_dim_x_,
			const LumiFit::LmdDimension& data_dim_y_,
			unsigned int number_of_threads_);
	virtual ~PndLmdDifferentialSmearingConvolutionModel2D();

	void initModelParameters();

	void injectModelParameter(shared_ptr<ModelPar> model_param);

	double eval(const double *x) const;

	void updateDomain();
};

#endif /* PNDLMDDIFFERENTIALSMEARINGCONVOLUTIONMODEL2D_H_ */
