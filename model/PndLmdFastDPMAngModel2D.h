#ifndef PNDLMDFASTDPMANGMODEL2D_H_
#define PNDLMDFASTDPMANGMODEL2D_H_

#include "core/Model2D.h"
#include "PndLmdDPMAngModel1D.h"

class PndLmdFastDPMAngModel2D: public Model2D {
	struct binrange {
		unsigned x_bin_low;
		unsigned x_bin_high;
		unsigned y_bin_low;
		unsigned y_bin_high;
	};

	shared_ptr<Model> dpm_model_1d;

	shared_ptr<ModelPar> tilt_x;
	shared_ptr<ModelPar> tilt_y;

	LumiFit::LmdDimension data_dim_x;
	LumiFit::LmdDimension data_dim_y;

	double **model_grid;

	unsigned int nthreads;
	std::vector<binrange> list_of_bin_ranges;

	double calculateThetaFromTiltedSystem(const double theta,
			const double phi) const;

	double calculateJacobianDeterminant(const double theta,
			const double phi) const;

	void generateModelGrid2D(const binrange &br);

	/*	void determineSmearGridBinning();

	 void generateSmearWeightGrid2D();*/
	void generateModelGrid2D();
	//void generateDivergenceSmearedModelGrid2D();

public:
	PndLmdFastDPMAngModel2D(std::string name_,
			shared_ptr<PndLmdDPMAngModel1D> dpm_model_1d_,
			const LumiFit::LmdDimension& data_dim_x_,
			const LumiFit::LmdDimension& data_dim_y_,
			unsigned int number_of_threads_);
	virtual ~PndLmdFastDPMAngModel2D();

	virtual void initModelParameters();

	double eval(const double *x) const;

	virtual void updateDomain();
};

#endif /* PNDLMDFASTDPMANGMODEL2D_H_ */
