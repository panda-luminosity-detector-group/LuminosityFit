/*
 * PndLmdDPMModel2D.cxx
 *
 *  Created on: Jan 17, 2013
 *      Author: steve
 */

#include "PndLmdFastDPMAngModel2D.h"

#include "TMath.h"
#include "TVector3.h"

#include <boost/thread.hpp>

PndLmdFastDPMAngModel2D::PndLmdFastDPMAngModel2D(std::string name_,
		shared_ptr<PndLmdDPMAngModel1D> dpm_model_1d_,
		const LumiFit::LmdDimension& data_dim_x_,
		const LumiFit::LmdDimension& data_dim_y_, unsigned int number_of_threads_) :
		Model2D(name_), dpm_model_1d(dpm_model_1d_), data_dim_x(data_dim_x_), data_dim_y(
				data_dim_y_), nthreads(number_of_threads_) {
	initModelParameters();
	this->addModelToList(dpm_model_1d);

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

PndLmdFastDPMAngModel2D::~PndLmdFastDPMAngModel2D() {
	for (unsigned int i = 0; i < data_dim_x.bins; i++) {
		delete (model_grid[i]);
	}
	delete (model_grid);
}

void PndLmdFastDPMAngModel2D::initModelParameters() {
	tilt_x = getModelParameterSet().addModelParameter("tilt_x");
	tilt_x->setValue(0.0);
	tilt_y = getModelParameterSet().addModelParameter("tilt_y");
	tilt_y->setValue(0.0);
}

double PndLmdFastDPMAngModel2D::calculateThetaFromTiltedSystem(
		const double thetax, const double thetay) const {

	/*double x = tan(tilt_x->getValue());
	 double y = tan(tilt_y->getValue());
	 TVector3 tilt(x, y, 1.0);
	 TVector3 rotate_axis(y, -x, 0.0);
	 double theta = atan(sqrt(pow(thetax, 2.0) + pow(thetay, 2.0)));
	 double phi = atan2(thetay, thetax);
	 TVector3 measured_direction(sin(theta) * cos(phi), sin(theta) * sin(phi),
	 cos(theta));
	 measured_direction.Rotate(tilt.Theta(), rotate_axis);
	 return std::make_pair(measured_direction.Theta(), measured_direction.Phi());*/

	double tilted_thetax = thetax - tilt_x->getValue();
	double tilted_thetay = thetay - tilt_y->getValue();
	return sqrt(pow(tilted_thetax, 2.0) + pow(tilted_thetay, 2.0));
	/*return std::make_pair(sqrt(pow(tilted_thetax, 2.0) + pow(tilted_thetay, 2.0)),
	 atan2(tilted_thetay, tilted_thetax));*/
}

double PndLmdFastDPMAngModel2D::calculateJacobianDeterminant(
		const double thetax, const double thetay) const {
	/*double e_m = 1.0 * 1e-16; // machine precision
	 double htx = pow(e_m, 0.33) * thetax;
	 double hty = pow(e_m, 0.33) * thetay;

	 std::pair<double, double> shift_plus_ht = calculateThetaFromTiltedSystem(
	 thetax + htx, thetay);
	 std::pair<double, double> shift_min_ht = calculateThetaFromTiltedSystem(
	 thetax - htx, thetay);
	 std::pair<double, double> shift_plus_hp = calculateThetaFromTiltedSystem(
	 thetax, thetay + hty);
	 std::pair<double, double> shift_min_hp = calculateThetaFromTiltedSystem(
	 thetax, thetay - hty);

	 double j11 = (shift_plus_ht.first - shift_min_ht.first) / (2 * htx);
	 double j12 = (shift_plus_hp.first - shift_min_hp.first) / (2 * hty);
	 double j21 = (shift_plus_ht.second - shift_min_ht.second) / (2 * htx);
	 double j22 = (shift_plus_hp.second - shift_min_hp.second) / (2 * hty);

	 return j11 * j22 - j21 * j12;*/

	// analytic formula
	double xypowsum = pow(thetax - tilt_x->getValue(), 2.0)
			+ pow(thetay - tilt_y->getValue(), 2.0);

	return 1.0 / (sqrt(xypowsum) * (1 + xypowsum));
}

/*void PndLmdFastDPMAngModel2D::determineSmearGridBinning() {
 //delete old grid first
 if (smear_grid) {
 for (unsigned int ix = 0; ix < (smear_grid_bins_x / 2) * 2 + 1; ix++) {
 delete (smear_grid[ix]);
 }
 delete (smear_grid);
 }

 // then determine new binning, which has to be of the same size as for the data.
 // that enables us to just shift the grids on top of each other
 smear_grid_bins_x = divergence_model->getVar1DomainRange() / dim_x.bin_size;
 smear_grid_bins_y = divergence_model->getVar2DomainRange() / dim_y.bin_size;

 smear_grid = new double*[(smear_grid_bins_x / 2) * 2 + 1];
 for (unsigned int ix = 0; ix < (smear_grid_bins_x / 2) * 2 + 1; ix++) {
 smear_grid[ix] = new double[(smear_grid_bins_y / 2) * 2 + 1];
 }
 }

 void PndLmdFastDPMAngModel2D::generateSmearWeightGrid2D() {
 // loop over divergence grid
 if (divergence_model) {
 determineSmearGridBinning();

 double tilt_value_x;
 double tilt_value_y;
 double div_binsize_x = divergence_model->getVar1DomainRange()
 / smear_grid_bins_x;
 double div_binsize_y = divergence_model->getVar2DomainRange()
 / smear_grid_bins_y;

 std::vector<DataStructs::DimensionRange> smear_model_integral_range;
 DataStructs::DimensionRange temp;
 smear_model_integral_range.push_back(temp);
 smear_model_integral_range.push_back(temp);

 for (int smear_ix = -smear_grid_bins_x / 2;
 smear_ix <= smear_grid_bins_x / 2; smear_ix++) {
 tilt_value_x = divergence_model->getVar1DomainLowerBound()
 + div_binsize_x * (0.5 + smear_ix + smear_grid_bins_x / 2);
 for (int smear_iy = -smear_grid_bins_y; smear_iy <= smear_grid_bins_y / 2;
 smear_iy++) {
 tilt_value_y = divergence_model->getVar2DomainLowerBound()
 + div_binsize_y * (0.5 + smear_iy + smear_grid_bins_y / 2);

 smear_model_integral_range[0].range_low = tilt_value_x
 - 0.5 * div_binsize_x;
 smear_model_integral_range[0].range_high = tilt_value_x
 + 0.5 * div_binsize_x;
 smear_model_integral_range[1].range_low = tilt_value_y
 - 0.5 * div_binsize_y;
 smear_model_integral_range[1].range_high = tilt_value_y
 + 0.5 * div_binsize_y;

 // integrate the smearing model (has to  be normalized overall)
 // over this bin in the divergence coordinates
 smear_grid[smear_ix][smear_iy] = divergence_model->Integral(
 smear_model_integral_range, 1e-2);
 }
 }
 }
 }*/

void PndLmdFastDPMAngModel2D::generateModelGrid2D() {
	// create threads and let them evaluate a part of the data
	boost::thread_group threads;

	for (unsigned int i = 0; i < nthreads; i++) {
		threads.create_thread(
				boost::bind(&PndLmdFastDPMAngModel2D::generateModelGrid2D, this,
						boost::cref(list_of_bin_ranges[i])));
	}

	threads.join_all();
}

void PndLmdFastDPMAngModel2D::generateModelGrid2D(const binrange &br) {
	double x[2];
	for (unsigned int ix = br.x_bin_low; ix < br.x_bin_high; ix++) {
		x[0] = data_dim_x.dimension_range.getRangeLow()
				+ data_dim_x.bin_size * (0.5 + ix);
		for (unsigned int iy = br.y_bin_low; iy < br.y_bin_high; iy++) {
			x[1] = data_dim_y.dimension_range.getRangeLow()
					+ data_dim_y.bin_size * (0.5 + iy);

			double jaco = calculateJacobianDeterminant(x[0], x[1]);
			double theta_tilted = calculateThetaFromTiltedSystem(x[0], x[1]);

			/*std::cout << "measured thetax, thetay: " << x[0] << "," << x[1]
			 << " -> transforms to evaluated theta of: " << theta_tilted
			 << std::endl;
			 std::cout << "jacobian: " << jaco << std::endl;
			 std::cout << "model value: " << dpm_model_1d->eval(&theta_tilted)
			 << std::endl;*/

			model_grid[ix][iy] = jaco * dpm_model_1d->eval(&theta_tilted)
					/ (2.0 * TMath::Pi());
		}
	}
}

double PndLmdFastDPMAngModel2D::eval(const double *x) const {
	/*double jaco = calculateJacobianDeterminant(x[0], x[1]);
	 double theta_tilted = calculateThetaFromTiltedSystem(x[0], x[1]);

	 /*std::cout << "measured thetax, thetay: " << x[0] << "," << x[1]
	 << " -> transforms to evaluated theta of: " << theta_tilted << std::endl;
	 std::cout << "jacobian: " << jaco << std::endl;
	 std::cout << "model value: " << dpm_model_1d->eval(&theta_tilted)
	 << std::endl

	 return jaco * dpm_model_1d->eval(&theta_tilted) / (2.0 * TMath::Pi());*/

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

void PndLmdFastDPMAngModel2D::updateDomain() {
	generateModelGrid2D();
}
