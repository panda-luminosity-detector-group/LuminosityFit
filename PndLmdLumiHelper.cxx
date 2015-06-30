/*
 * PndLmdLumiHelper.cxx
 *
 *  Created on: Jun 27, 2012
 *      Author: steve
 */

#include "fit/ModelFitResult.h"
#include "core/ModelStructs.h"

#include "PndLmdLumiHelper.h"
#include "data/PndLmdHistogramData.h"
#include "model/PndLmdDPMAngModel1D.h"
#include "model/PndLmdDPMModelParametrization.h"

#include "TH1D.h"
#include "TH2D.h"

PndLmdLumiHelper::PndLmdLumiHelper() {
}

double PndLmdLumiHelper::getMomentumTransferFromTheta(double plab,
		double theta) {
	PndLmdDPMAngModel1D model("dpm_angular_1d", LumiFit::ALL);
	shared_ptr<Parametrization> para(
			new PndLmdDPMModelParametrization(model.getModelParameterSet()));
	model.getModelParameterHandler().registerParametrizations(
			model.getModelParameterSet(), para);
	model.getModelParameterSet().setModelParameterValue("p_lab", plab);
	((Model1D*) &model)->init();
	return -model.getMomentumTransferFromTheta(theta);
}

double PndLmdLumiHelper::calcHistIntegral(const TH1D* hist,
		std::vector<DataStructs::DimensionRange> range) {
	// just determine the bin range over which we have to calculate
	if (range.size() < 1)
		return 0.0;
	int bin_low = 1; // first bin is underflow
	int bin_high = hist->GetNbinsX();
	for (int i = 0; i < hist->GetNbinsX(); i++) {
		if (hist->GetBinCenter(i) - hist->GetBinWidth(i) / 2 < range[0].range_low
				&& hist->GetBinCenter(i) + hist->GetBinWidth(i) / 2
						> range[0].range_low) {
			bin_low = i;
		}
		if (hist->GetBinCenter(i) - hist->GetBinWidth(i) / 2 < range[0].range_high
				&& hist->GetBinCenter(i) + hist->GetBinWidth(i) / 2
						> range[0].range_high) {
			bin_high = i;
		}
	}
	return hist->Integral(bin_low, bin_high, "width");
}

double PndLmdLumiHelper::calcHistIntegral(const TH2D* hist,
		std::vector<DataStructs::DimensionRange> range) {
	// just determine the bin range over which we have to calculate
	if (range.size() < 2)
		return 0.0;
	int bin_low_x = 1; // first bin is underflow
	int bin_high_x = hist->GetNbinsX();
	for (int i = 0; i < hist->GetNbinsX(); i++) {
		if (hist->GetXaxis()->GetBinCenter(i) - hist->GetXaxis()->GetBinWidth(i) / 2
				< range[0].range_low
				&& hist->GetXaxis()->GetBinCenter(i)
						+ hist->GetXaxis()->GetBinWidth(i) / 2 > range[0].range_low) {
			bin_low_x = i;
		}
		if (hist->GetXaxis()->GetBinCenter(i) - hist->GetXaxis()->GetBinWidth(i) / 2
				< range[0].range_high
				&& hist->GetXaxis()->GetBinCenter(i)
						+ hist->GetXaxis()->GetBinWidth(i) / 2 > range[0].range_high) {
			bin_high_x = i;
		}
	}
	int bin_low_y = 1; // first bin is underflow
	int bin_high_y = hist->GetNbinsY();
	for (int i = 0; i < hist->GetNbinsY(); i++) {
		if (hist->GetYaxis()->GetBinCenter(i) - hist->GetYaxis()->GetBinWidth(i) / 2
				< range[1].range_low
				&& hist->GetYaxis()->GetBinCenter(i)
						+ hist->GetYaxis()->GetBinWidth(i) / 2 > range[1].range_low) {
			bin_low_y = i;
		}
		if (hist->GetYaxis()->GetBinCenter(i) - hist->GetYaxis()->GetBinWidth(i) / 2
				< range[1].range_high
				&& hist->GetYaxis()->GetBinCenter(i)
						+ hist->GetYaxis()->GetBinWidth(i) / 2 > range[1].range_high) {
			bin_high_y = i;
		}
	}
	return hist->Integral(bin_low_x, bin_high_x, bin_low_y, bin_high_y, "width");
}


std::map<double, ModelFitResult*> PndLmdLumiHelper::checkFitParameters(
		const std::map<PndLmdHistogramData*, ModelFitResult*> &fit_results) const {
// remove all points that are bogus
	std::map<double, ModelFitResult*> return_map;
// loop over all entries of the map
	for (std::map<PndLmdHistogramData*, ModelFitResult*>::const_iterator it =
			fit_results.begin(); it != fit_results.end(); it++) {
		const std::set<ModelStructs::minimization_parameter> &fit_parameters =
				it->second->getFitParameters();
		bool add = true;
		for (std::set<ModelStructs::minimization_parameter>::const_iterator fit_param =
				fit_parameters.begin(); fit_param != fit_parameters.end();
				fit_param++) {
			/*if (fit_param->name.find("red_chi2") < fit_param->name.size()) {
			 if (std::fabs(fit_param->value) > 1.35) {
			 add = false;
			 break;
			 }
			 }
			 if (fit_param->name.find("ratio_narrow_wide") < fit_param->name.size()) {
			 std::cout << fit_param->value << std::endl;
			 if (std::fabs(fit_param->value) < 0.0) {
			 add = false;
			 break;
			 }
			 }*/
		}
		if (add) {
			const std::set<LumiFit::LmdDimension>& selection_dimensions =
					it->first->getSelectorSet();
			std::set<LumiFit::LmdDimension>::iterator selection = std::find_if(
					selection_dimensions.begin(), selection_dimensions.end(),
					findByDimensionType(LumiFit::THETA));

			if (selection != selection_dimensions.end()) {
				return_map.insert(
						std::make_pair(selection->dimension_range.getDimensionMean(),
								it->second));
			}
		}
	}
	return return_map;
}
