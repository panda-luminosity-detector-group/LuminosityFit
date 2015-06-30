/*
 * PndLmdDivergenceSmearingModel2D.cxx
 *
 *  Created on: Nov 21, 2014
 *      Author: steve
 */

#include "LumiFitStructs.h"
#include "model/PndLmdDivergenceSmearingModel2D.h"

#include <limits>

PndLmdDivergenceSmearingModel2D::PndLmdDivergenceSmearingModel2D(
		shared_ptr<Model2D> divergence_model_,
		const LumiFit::LmdDimension& data_dim_x_,
		const LumiFit::LmdDimension& data_dim_y_) :
		divergence_model(divergence_model_), data_dim_x(data_dim_x_), data_dim_y(
				data_dim_y_) {
}

PndLmdDivergenceSmearingModel2D::~PndLmdDivergenceSmearingModel2D() {
}

std::pair<double, double> PndLmdDivergenceSmearingModel2D::getBinsizes() const {
	return std::make_pair(data_dim_x.bin_size, data_dim_y.bin_size);
}

const std::vector<DifferentialCoordinateContribution>& PndLmdDivergenceSmearingModel2D::getListOfContributors(
		const double *x) const {
	return list_of_contributors;
}

void PndLmdDivergenceSmearingModel2D::generate2DDivergenceMap() {
	// we require a homogeneous bining for the divergence grid for this
	// algorithm to work!

	/*double prim_dim_binsize = data_dim_x.bin_size;
	 double sec_dim_binsize = data_dim_y.bin_size;

	 unsigned int div_binning_x = 21;
	 unsigned int div_binning_y = 21;

	 double div_bin_size_x = divergence_model->getVar1DomainRange()
	 / div_binning_x;
	 double div_bin_size_y = divergence_model->getVar2DomainRange()
	 / div_binning_y;

	 std::vector<DataStructs::DimensionRange> int_range(2);

	 double onehalf = 0.5 - std::numeric_limits<double>::epsilon();
	 double rec_signum_x(-0.5);
	 double rec_signum_y(-0.5);

	 std::vector<DifferentialCoordinateContribution> temp_list_of_contributors;
	 temp_list_of_contributors.reserve(4 * div_binning_x * div_binning_y);

	 std::cout<<"calculating divergence contributions..."<<std::endl;

	 // loop over the divergence bining
	 for (unsigned int ibinx = 0; ibinx < div_binning_x; ibinx++) {
	 int_range[0].range_low = divergence_model->getVar1DomainLowerBound()
	 + ibinx * div_bin_size_x;
	 int_range[0].range_high = divergence_model->getVar1DomainLowerBound()
	 + (ibinx + 1) * div_bin_size_x;

	 double signum_x_low(-onehalf);
	 if (int_range[0].range_low < 0.0)
	 signum_x_low = onehalf;
	 double signum_x_high(-onehalf);
	 if (int_range[0].range_high < 0.0)
	 signum_x_high = onehalf;

	 for (unsigned int ibiny = 0; ibiny < div_binning_y; ibiny++) {
	 int_range[1].range_low = divergence_model->getVar2DomainLowerBound()
	 + ibiny * div_bin_size_y;
	 int_range[1].range_high = divergence_model->getVar2DomainLowerBound()
	 + (ibiny + 1) * div_bin_size_y;

	 double signum_y_low(-onehalf);
	 if (int_range[1].range_low < 0.0)
	 signum_y_low = onehalf;
	 double signum_y_high(-onehalf);
	 if (int_range[1].range_high < 0.0)
	 signum_y_high = onehalf;

	 // calculate integral over the divergence bin
	 double smearing_probability = divergence_model->Integral(int_range, 1e-3);

	 LumiFit::BinDimension reso_bin(int_range[0], int_range[1]);

	 LumiFit::BinDimension reco_bin;

	 // calculated boundaries
	 int recbinx_min = (reso_bin.x_range.range_low / prim_dim_binsize
	 - signum_x_low);
	 int recbinx_max = (reso_bin.x_range.range_high / prim_dim_binsize
	 - signum_x_high);
	 int recbiny_min = (reso_bin.y_range.range_low / sec_dim_binsize
	 - signum_y_low);
	 int recbiny_max = (reso_bin.y_range.range_high / sec_dim_binsize
	 - signum_y_high);

	 /*std::cout << recbinx_min << " " << reso_bin.x_range.range_low << " "
	 << prim_dim_binsize << " " << signum_x_low << std::endl;
	 std::cout << recbinx_max << " " << reso_bin.x_range.range_high << " "
	 << prim_dim_binsize << " " << signum_x_high << std::endl;
	 std::cout << recbiny_min << " " << reso_bin.y_range.range_low << " "
	 << sec_dim_binsize << " " << signum_y_low << std::endl;
	 std::cout << recbiny_max << " " << reso_bin.y_range.range_high << " "
	 << sec_dim_binsize << " " << signum_y_high << std::endl;*/

	/*	for (int irecbinx = recbinx_min; irecbinx <= recbinx_max; ++irecbinx) {
	 for (int irecbiny = recbiny_min; irecbiny <= recbiny_max; ++irecbiny) {
	 //reco bin information
	 reco_bin.x_range.range_low = (rec_signum_x + irecbinx)
	 * prim_dim_binsize;
	 reco_bin.x_range.range_high = reco_bin.x_range.range_low
	 + prim_dim_binsize;
	 reco_bin.y_range.range_low = (rec_signum_y + irecbiny)
	 * sec_dim_binsize;
	 reco_bin.y_range.range_high = reco_bin.y_range.range_low
	 + sec_dim_binsize;

	 // calculate overlap with elastic data reco bins
	 double weight = LumiFit::calculateBinOverlap(reco_bin, reso_bin);

	 DifferentialCoordinateContribution dcc(irecbinx, irecbiny,
	 weight * smearing_probability);

	 temp_list_of_contributors.push_back(dcc);
	 }
	 }
	 }
	 }*/

	// we require a homogeneous bining for the divergence grid for this
	// algorithm to work!
	double div_bin_size_x = data_dim_x.bin_size;
	double div_bin_size_y = data_dim_y.bin_size;

	int div_bining_x = divergence_model->getVar1DomainRange()
			/ div_bin_size_x / 2.0;
	int div_bining_y = divergence_model->getVar2DomainRange()
			/ div_bin_size_y / 2.0;

	std::vector<DataStructs::DimensionRange> int_range(2);

	list_of_contributors.clear();
	list_of_contributors.reserve(4 * div_bining_x * div_bining_y);

	//std::cout << "calculating divergence contributions..." << std::endl;
	//std::cout << "using bining: " << div_bining_x << " : " << div_bining_y
	//		<< std::endl;

	std::set<DifferentialCoordinateContribution, PndLmdDivergenceSmearingModel2D::IntPairLess> temp_contribution_list;

	// loop over the divergence bining
	for (int ibinx = -div_bining_x; ibinx <= div_bining_x; ++ibinx) {
		int_range[0].range_low = (ibinx - 0.5) * div_bin_size_x;
		int_range[0].range_high = (ibinx + 0.5) * div_bin_size_x;

		for (int ibiny = -div_bining_y; ibiny <= div_bining_y; ++ibiny) {
			int_range[1].range_low = (ibiny - 0.5) * div_bin_size_y;
			int_range[1].range_high = (ibiny + 0.5) * div_bin_size_y;

			// calculate integral over the divergence bin
			double smearing_probability = divergence_model->Integral(int_range, 1e-4);

			DifferentialCoordinateContribution dcc(ibinx, ibiny,
					smearing_probability);

			temp_contribution_list.insert(dcc);
		}
	}

	//std::cout << "sorting " << list_of_contributors.size()
	//		<< " divergence contributions..." << std::endl;

	std::set<DifferentialCoordinateContribution, PndLmdDivergenceSmearingModel2D::IntPairLess>::const_iterator contribution_iter;
	for(contribution_iter = temp_contribution_list.begin(); contribution_iter != temp_contribution_list.end(); ++contribution_iter)
		list_of_contributors.push_back(*contribution_iter);
	//std::sort(list_of_contributors.begin(), list_of_contributors.end(),
	//		IntPairLess());

	/*list_of_contributors.clear();
	 list_of_contributors.reserve(temp_list_of_contributors.size());

	 std::cout << "merging divergence contributions..." << std::endl;

	 if (temp_list_of_contributors.size() > 0) {
	 unsigned int current_position(0);
	 list_of_contributors.push_back(temp_list_of_contributors[0]);
	 for (unsigned int i = 1; i < temp_list_of_contributors.size(); ++i) {
	 const DifferentialCoordinateContribution &temp_contribution =
	 temp_list_of_contributors[i];
	 if (temp_contribution == list_of_contributors[current_position]) {
	 list_of_contributors[current_position].contribution_factor +=
	 temp_contribution.contribution_factor;
	 } else {
	 list_of_contributors.push_back(temp_list_of_contributors[i]);
	 ++current_position;
	 }
	 }
	 }*/
	// in case we reserved more than we needed free the remainder
	// comment this line for more speed...
	list_of_contributors.resize(list_of_contributors.size());
	//std::cout << "done!" << std::endl;
}

void PndLmdDivergenceSmearingModel2D::updateSmearingModel() {
	divergence_model->updateDomain();
	generate2DDivergenceMap();
}
