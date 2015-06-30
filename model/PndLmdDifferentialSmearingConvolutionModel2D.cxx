#include "PndLmdDifferentialSmearingConvolutionModel2D.h"

#include <cmath>
#include <iostream>

#include <boost/thread.hpp>

#include "TMath.h"

PndLmdDifferentialSmearingConvolutionModel2D::PndLmdDifferentialSmearingConvolutionModel2D(
		std::string name_, shared_ptr<Model2D> unsmeared_model_,
		shared_ptr<PndLmdDivergenceSmearingModel2D> smearing_model_,
		const LumiFit::LmdDimension& data_dim_x_,
		const LumiFit::LmdDimension& data_dim_y_, unsigned int number_of_threads_) :
		Model2D(name_), data_dim_x(data_dim_x_), data_dim_y(data_dim_y_), nthreads(
				number_of_threads_) {

	unsmeared_model = unsmeared_model_;
	smearing_model = smearing_model_;

	binsizes = smearing_model->getBinsizes();
	area_xy = binsizes.first * binsizes.second;

	addModelToList(unsmeared_model);

	setVar1Domain(-TMath::Pi(), TMath::Pi());
	setVar2Domain(-TMath::Pi(), TMath::Pi());

	unsigned int bins_per_thread_x(data_dim_x.bins / nthreads);
	unsigned int bins_per_thread_y(data_dim_y.bins / nthreads);

	bool use_x_splitting(true);
	if (data_dim_x.bins % nthreads != 0) {
		if (data_dim_y.bins % nthreads == 0)
			use_x_splitting = false;
		else {
			if (data_dim_x.bins < 0 && data_dim_y.bins < 0) {
				if (data_dim_y.bins > data_dim_x.bins) {
					use_x_splitting = false;
					nthreads = data_dim_y.bins;
				} else {
					nthreads = data_dim_x.bins;
				}
			} else {
				if (data_dim_y.bins % nthreads > data_dim_y.bins % nthreads) {
					use_x_splitting = false;
					bins_per_thread_y++;
				}

				else {
					bins_per_thread_x++;
				}
			}
		}
	}
	binrange br;
	br.x_bin_low = 0;
	br.x_bin_high = data_dim_x.bins;
	br.y_bin_low = 0;
	br.y_bin_high = data_dim_y.bins;
	for (unsigned int i = 0; i < nthreads; ++i) {
		if (use_x_splitting) {
			br.x_bin_low = i * bins_per_thread_x;
			br.x_bin_high = (i + 1) * bins_per_thread_x;
			if (i == nthreads - 1)
				br.x_bin_high = data_dim_x.bins;
		} else {
			br.y_bin_low = i * bins_per_thread_y;
			br.y_bin_high = (i + 1) * bins_per_thread_y;
			if (i == nthreads - 1)
				br.y_bin_high = data_dim_y.bins;
		}
		list_of_bin_ranges.push_back(br);
	}

	model_grid = new double*[data_dim_x.bins];
	for (unsigned int i = 0; i < data_dim_x.bins; i++) {
		model_grid[i] = new double[data_dim_y.bins];
	}
}

PndLmdDifferentialSmearingConvolutionModel2D::~PndLmdDifferentialSmearingConvolutionModel2D() {
	for (unsigned int i = 0; i < data_dim_x.bins; i++) {
		delete (model_grid[i]);
	}
	delete (model_grid);
}

void PndLmdDifferentialSmearingConvolutionModel2D::initModelParameters() {
}

void PndLmdDifferentialSmearingConvolutionModel2D::injectModelParameter(
		shared_ptr<ModelPar> model_param) {
	getModelParameterSet().addModelParameter(model_param);
}

void PndLmdDifferentialSmearingConvolutionModel2D::generateModelGrid2D() {
	//std::cout << "generating divergence smeared grid..." << std::endl;
	// create threads and let them evaluate a part of the data
	boost::thread_group threads;

	for (unsigned int i = 0; i < nthreads; i++) {
		threads.create_thread(
				boost::bind(
						&PndLmdDifferentialSmearingConvolutionModel2D::generateModelGrid2D,
						this, boost::cref(list_of_bin_ranges[i])));
	}

	threads.join_all();
	//std::cout << "done!" << std::endl;
}

void PndLmdDifferentialSmearingConvolutionModel2D::generateModelGrid2D(
		const binrange &br) {
	double x[2];
	for (unsigned int ix = br.x_bin_low; ix < br.x_bin_high; ix++) {
		x[0] = data_dim_x.dimension_range.getRangeLow()
				+ data_dim_x.bin_size * (0.5 + ix);
		for (unsigned int iy = br.y_bin_low; iy < br.y_bin_high; iy++) {
			x[1] = data_dim_y.dimension_range.getRangeLow()
					+ data_dim_y.bin_size * (0.5 + iy);

			double value = 0.0;

			const std::vector<DifferentialCoordinateContribution> &mc_element_contributors =
					smearing_model->getListOfContributors(x);

			std::vector<DifferentialCoordinateContribution>::const_iterator mc_element_it;
			for (mc_element_it = mc_element_contributors.begin();
					mc_element_it != mc_element_contributors.end(); ++mc_element_it) {
				double xx[2];
				xx[0] = x[0] + binsizes.first * mc_element_it->coordinate_delta.first;
				xx[1] = x[1] + binsizes.second * mc_element_it->coordinate_delta.second;
				double integral_unsmeared_model = unsmeared_model->evaluate(xx);

				//integral_unsmeared_model = integral_unsmeared_model * area_xy;
				value = value
						+ integral_unsmeared_model * mc_element_it->contribution_factor;

				/*if (fabs(x[0] + 0.00615) < 0.0001 && fabs(x[1] + 0.00705) < 0.0001) {
				 std::cout << xx[0] << ", " << xx[1] << std::endl;
				 std::cout << integral_unsmeared_model << " " << mc_element_it->second
				 << std::endl;
				 }*/
			}

			/*if (fabs(x[0] + 0.00615) < 0.0001 && fabs(x[1] + 0.00705) < 0.0001) {
			 std::cout << x[0] << ", " << x[1] << " value: " << value << std::endl;
			 std::cout << "contribution size: " << mc_element_contributors.size()
			 << std::endl;
			 }*/

			model_grid[ix][iy] = value;
		}
	}
}

double PndLmdDifferentialSmearingConvolutionModel2D::eval(
		const double *x) const {
	int ix = (x[0] - data_dim_x.dimension_range.getRangeLow())
			/ data_dim_x.bin_size;
	int iy = (x[1] - data_dim_y.dimension_range.getRangeLow())
			/ data_dim_y.bin_size;

	/*std::cout << ix << " " << iy << " " << data_dim_x.bins << " "
	 << data_dim_y.bins << " " << x[0] << " "
	 << data_dim_x.dimension_range.getRangeLow() << " - "
	 << data_dim_x.dimension_range.getRangeHigh() << " " << x[1] << " "
	 << data_dim_y.dimension_range.getRangeLow() << " - "
	 << data_dim_y.dimension_range.getRangeHigh() << std::endl;*/

	if (ix >= data_dim_x.bins || iy >= data_dim_y.bins || ix < 0 || iy < 0)
		return 0.0;
	return model_grid[ix][iy];
}

void PndLmdDifferentialSmearingConvolutionModel2D::updateDomain() {
	smearing_model->updateSmearingModel();
	generateModelGrid2D();
}
