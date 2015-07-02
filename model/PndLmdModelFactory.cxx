/*
 * PndLmdModelFactory.cxx
 *
 *  Created on: Dec 18, 2012
 *      Author: steve
 */

#include "PndLmdModelFactory.h"
#include "fit/ModelFitResult.h"
#include "fit/PndLmdLumiFitResult.h"
#include "data/PndLmdAcceptance.h"
#include "PndLmdDPMMTModel1D.h"
#include "PndLmdDPMAngModel1D.h"
#include "PndLmdDPMAngModel2D.h"
#include "PndLmdFastDPMAngModel2D.h"
#include "PndLmdROOTDataModel1D.h"
#include "PndLmdROOTDataModel2D.h"
#include "models1d/GaussianModel1D.h"
#include "models2d/GaussianModel2D.h"
#include "models2d/DataModel2D.h"
#include "models1d/DoubleGaussianModel1D.h"
#include "AsymmetricGaussianModel1D.h"
#include "PndLmdDivergenceSmearingModel2D.h"

#include "PndLmdSignalBackgroundModel1D.h"
#include "PndLmdBackgroundModel1D.h"

#include "operators2d/integration/SimpleIntegralStrategy2D.h"
#include "operators1d/ProductModel1D.h"
#include "operators2d/ProductModel2D.h"
#include "operators1d/convolution/NumericConvolutionModel1D.h"
#include "operators1d/convolution/SmearingConvolutionModel1D.h"
#include "model/PndLmdSmearingConvolutionModel2D.h"
#include "model/PndLmdDifferentialSmearingConvolutionModel2D.h"
#include "PndLmdDPMModelParametrization.h"

#include "ui/PndLmdDataFacade.h"
#include "data/PndLmdAngularData.h"

#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "TEfficiency.h"
#include "TH2.h"
#include "TCanvas.h"

PndLmdModelFactory::PndLmdModelFactory() {

}

PndLmdModelFactory::~PndLmdModelFactory() {

}

shared_ptr<PndLmdSmearingModel2D> PndLmdModelFactory::generate2DSmearingModel(
		const PndLmdHistogramData &data) const {
	std::map<std::pair<double, double>, std::map<LumiFit::BinDimension, double> > resolution_map;

	TH2D* recohist2d = data.get2DHistogram();

	int prim_dim_bins = data.getPrimaryDimension().bins;
	double prim_dim_binsize = data.getPrimaryDimension().bin_size;
	double prim_dim_start =
			data.getPrimaryDimension().dimension_range.getRangeLow();

	int sec_dim_bins = data.getSecondaryDimension().bins;
	double sec_dim_binsize = data.getSecondaryDimension().bin_size;
	double sec_dim_start =
			data.getSecondaryDimension().dimension_range.getRangeLow();

	double binning_factor = 1 / sec_dim_binsize / prim_dim_binsize;

	std::cout << "calculating detector smearing contributions..." << std::endl;

	// loop over all resolution objects
	for (unsigned int i = 0; i < resolutions.size(); i++) {
		//std::cout << "processing resolution " << i << std::endl;
		if (resolutions[i].getSecondaryDimension().is_active) {
			// try to find the selction dimension (mc values)
			LumiFit::LmdDimension prim_dim;
			LumiFit::LmdDimension sec_dim;

			bool found_primary_selection(false);
			bool found_secondary_selection(false);

			const std::set<LumiFit::LmdDimension>& selection_set =
					resolutions[i].getSelectorSet();
			std::set<LumiFit::LmdDimension>::const_iterator selection_set_it;
			for (selection_set_it = selection_set.begin();
					selection_set_it != selection_set.end(); selection_set_it++) {
				if (selection_set_it->dimension_options.dimension_type
						== data.getPrimaryDimension().dimension_options.dimension_type) {
					prim_dim = *selection_set_it;
					found_primary_selection = true;
				} else if (selection_set_it->dimension_options.dimension_type
						== data.getSecondaryDimension().dimension_options.dimension_type) {
					sec_dim = *selection_set_it;
					found_secondary_selection = true;
				}
			}

			if (found_primary_selection && found_secondary_selection) {
				// ok we found the selection dimensions
				TH2D* hist2d = resolutions[i].get2DHistogram();
				// calculated normalization integral
				double hist_integral = hist2d->Integral();
				if (hist_integral <= 0.0)
					continue;

				double total_smearing_probability = 0.0;
				double total_smearing_probability_noweight = 0.0;
				int counter = 0;

				// loop over the resolution histogram bins
				for (unsigned int ibinx = 1; ibinx < hist2d->GetNbinsX() - 1; ibinx++) {
					for (unsigned int ibiny = 1; ibiny < hist2d->GetNbinsY() - 1;
							ibiny++) {
						// ok so now we just need to find the reco bins (there could be some overlap with
						// multiple reco bins) for each of the resolution bins and weight the full bin
						// probability according to the overlap
						if (hist2d->GetBinContent(ibinx, ibiny) > 0.0) {
							double smearing_probability = hist2d->GetBinContent(ibinx, ibiny)
									/ hist_integral * binning_factor;

							//total_smearing_probability_noweight += smearing_probability
							//		/ binning_factor;

							//std::cout << "smearing probability: " << smearing_probability
							//<< std::endl;

							LumiFit::BinDimension reso_bin;
							reso_bin.x_range.range_low =
									(prim_dim.dimension_range.getDimensionMean()
											+ hist2d->GetXaxis()->GetBinLowEdge(ibinx));
							reso_bin.x_range.range_high =
									(prim_dim.dimension_range.getDimensionMean()
											+ hist2d->GetXaxis()->GetBinUpEdge(ibinx));
							reso_bin.y_range.range_low =
									(sec_dim.dimension_range.getDimensionMean()
											+ hist2d->GetYaxis()->GetBinLowEdge(ibiny));
							reso_bin.y_range.range_high =
									(sec_dim.dimension_range.getDimensionMean()
											+ hist2d->GetYaxis()->GetBinUpEdge(ibiny));

							LumiFit::BinDimension reco_bin;

							// calculated boundaries
							int recbinx_min = (reso_bin.x_range.range_low - prim_dim_start)
									/ prim_dim_binsize + 1;
							int recbinx_max = (reso_bin.x_range.range_high - prim_dim_start)
									/ prim_dim_binsize + 1;
							int recbiny_min = (reso_bin.y_range.range_low - sec_dim_start)
									/ sec_dim_binsize + 1;
							int recbiny_max = (reso_bin.y_range.range_high - sec_dim_start)
									/ sec_dim_binsize + 1;

							//in case the resolution bin is further down the lower bound of the reco hist
							//just skip

							/*if (recbinx_max <= 0 || recbinx_min >= prim_dim_bins)
							 std::cout << recbinx_min << " " << recbinx_max << " "
							 << prim_dim_bins << std::endl;
							 if (recbiny_max <= 0 || recbiny_min >= sec_dim_bins)
							 std::cout << recbiny_min << " " << recbiny_max << " "
							 << sec_dim_bins << std::endl;*/

							if (recbinx_max <= 0 || recbinx_min >= prim_dim_bins)
								continue;
							if (recbiny_max <= 0 || recbiny_min >= sec_dim_bins)
								continue;

							/*std::cout << theta_start << " asdf " << phi_start << std::endl;
							 std::cout << recbinx_min << " = " << reso_bin.x_min << " "
							 << theta_start << " " << theta_binsize << std::endl;
							 std::cout << recbinx_max << " = " << reso_bin.x_max << " "
							 << theta_start << " " << theta_binsize << std::endl;
							 std::cout << recbiny_min << " = " << reso_bin.y_min << " "
							 << phi_start << " " << phi_binsize << std::endl;
							 std::cout << recbiny_max << " = " << reso_bin.y_max << " "
							 << phi_start << " " << phi_binsize << std::endl;*/

							for (unsigned int irecbinx = recbinx_min; irecbinx <= recbinx_max;
									irecbinx++) {
								for (unsigned int irecbiny = recbiny_min;
										irecbiny <= recbiny_max; irecbiny++) {
									// calculate overlap with elastic data reco bins

									reco_bin.x_range.range_low =
											recohist2d->GetXaxis()->GetBinLowEdge(irecbinx);
									reco_bin.x_range.range_high =
											recohist2d->GetXaxis()->GetBinUpEdge(irecbinx);
									reco_bin.y_range.range_low =
											recohist2d->GetYaxis()->GetBinLowEdge(irecbiny);
									reco_bin.y_range.range_high =
											recohist2d->GetYaxis()->GetBinUpEdge(irecbiny);

									double weight = LumiFit::calculateBinOverlap(reco_bin,
											reso_bin);

									double prim_var_rec = recohist2d->GetXaxis()->GetBinCenter(
											irecbinx);
									double sec_var_rec = recohist2d->GetYaxis()->GetBinCenter(
											irecbiny);

									//std::cout << "weight: " << weight << std::endl;

									if (resolution_map[std::make_pair(prim_var_rec, sec_var_rec)].find(
											LumiFit::BinDimension(prim_dim.dimension_range,
													sec_dim.dimension_range))
											== resolution_map[std::make_pair(prim_var_rec,
													sec_var_rec)].end()) {
										resolution_map[std::make_pair(prim_var_rec, sec_var_rec)][LumiFit::BinDimension(
												prim_dim.dimension_range, sec_dim.dimension_range)] =
												weight * smearing_probability;
									} else {
										resolution_map[std::make_pair(prim_var_rec, sec_var_rec)][LumiFit::BinDimension(
												prim_dim.dimension_range, sec_dim.dimension_range)] +=
												weight * smearing_probability;
									}

									total_smearing_probability += weight * smearing_probability
											/ binning_factor;
									counter++;
								}
							}
						}
					}
				}
				/*if (total_smearing_probability < 0.99) {
				 std::cout << "probability for "
				 << prim_dim.dimension_range.getDimensionMean() << ", "
				 << sec_dim.dimension_range.getDimensionMean() << std::endl;
				 std::cout << "total smearing probs: " << total_smearing_probability
				 << " " << total_smearing_probability_noweight << std::endl;
				 std::cout << counter << std::endl;
				 }*/
			}
		}
	}

	/*std::map<PndLmdLumiHelper::bin_dimension, std::pair<double, int> > test_map;

	 std::map<std::pair<double, double>,
	 std::map<PndLmdLumiHelper::bin_dimension, double> >::iterator it;

	 for (it = resolution_map.begin(); it != resolution_map.end(); it++) {
	 std::map<PndLmdLumiHelper::bin_dimension, double>::iterator it2;
	 for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
	 if (test_map.find(it2->first) == test_map.end())
	 test_map[it2->first] = std::make_pair(it2->second, 1);
	 else {
	 test_map[it2->first].first += it2->second;
	 test_map[it2->first].second++;
	 }
	 }
	 }

	 std::map<PndLmdLumiHelper::bin_dimension, std::pair<double, int> >::iterator it2;
	 for (it2 = test_map.begin(); it2 != test_map.end(); it2++) {
	 std::cout << "probability for " << it2->first.x_range.range_low << ", "
	 << it2->first.x_range.range_high << ": " << it2->second.second << " "
	 << it2->second.first << std::endl;
	 }*/

	// convert to a vector for faster access due to better caching
	std::cout
			<< "converting detector smearing contributions to vector for fast access..."
			<< std::endl;

	std::vector<RecoBinSmearingContributions> smearing_param;
	smearing_param.reserve(resolution_map.size());

	std::map<std::pair<double, double>, std::map<LumiFit::BinDimension, double> >::iterator iter;
	for (iter = resolution_map.begin(); iter != resolution_map.end(); ++iter) {
		RecoBinSmearingContributions rbsc;
		rbsc.reco_bin_x = iter->first.first;
		rbsc.reco_bin_y = iter->first.second;

		rbsc.contributor_coordinate_weight_list.reserve(iter->second.size());
		std::map<LumiFit::BinDimension, double>::iterator contribution_list_iter;
		for (contribution_list_iter = iter->second.begin();
				contribution_list_iter != iter->second.end();
				++contribution_list_iter) {
			ContributorCoordinateWeight cw;
			cw.bin_center_x =
					contribution_list_iter->first.x_range.getDimensionMean();
			cw.bin_center_y =
					contribution_list_iter->first.y_range.getDimensionMean();

			cw.area = contribution_list_iter->first.x_range.getDimensionLength()
					* contribution_list_iter->first.y_range.getDimensionLength();
			cw.smear_weight = contribution_list_iter->second;
			rbsc.contributor_coordinate_weight_list.push_back(cw);
		}
		smearing_param.push_back(rbsc);
	}
	std::cout << "done!" << std::endl;

	shared_ptr<PndLmdSmearingModel2D> detector_smearing_model(
			new PndLmdSmearingModel2D());

	detector_smearing_model->setSmearingParameterization(smearing_param);
	return detector_smearing_model;
}

double PndLmdModelFactory::getMomentumTransferFromTheta(double plab,
		double theta) const {
	PndLmdDPMAngModel1D model("dpm_angular_1d", LumiFit::ALL);
	shared_ptr<Parametrization> para(
			new PndLmdDPMModelParametrization(model.getModelParameterSet()));
	model.getModelParameterHandler().registerParametrizations(
			model.getModelParameterSet(), para);
	model.getModelParameterSet().setModelParameterValue("p_lab", plab);
	((Model1D*) &model)->init();
	return -model.getMomentumTransferFromTheta(theta);
}


shared_ptr<Model1D> PndLmdModelFactory::generate1DModel(
		const boost::property_tree::ptree& model_opt_ptree,
		const PndLmdAngularData& data) const {
	shared_ptr<Model1D> current_model;

	LumiFit::DPMElasticParts elastic_parts = LumiFit::StringToDPMElasticParts.at(
			model_opt_ptree.get<std::string>("dpm_elastic_parts"));

	if (model_opt_ptree.get<bool>("momentum_transfer_active")) {
		current_model.reset(new PndLmdDPMMTModel1D("dpm_mt_1d", elastic_parts));
		// set free parameters
	} else {
		current_model.reset(
				new PndLmdDPMAngModel1D("dpm_angular_1d", elastic_parts));
	}

	shared_ptr<Parametrization> dpm_parametrization(
			new PndLmdDPMModelParametrization(current_model->getModelParameterSet()));
	current_model->getModelParameterHandler().registerParametrizations(
			current_model->getModelParameterSet(), dpm_parametrization);

	if (model_opt_ptree.get<bool>("acceptance_correction_active")) { // with acceptance corr
		if (acceptance.getAcceptance1D()) {
			// translate acceptance interpolation option
			PndLmdROOTDataModel1D *data_model = new PndLmdROOTDataModel1D(
					"acceptance_1d");

			TVirtualPad *current_pad = gPad;
			TCanvas can;
			acceptance.getAcceptance1D()->Draw();
			can.Update();

			data_model->setGraph(acceptance.getAcceptance1D()->GetPaintedGraph());
			data_model->setIntpolType(
					LumiFit::StringToInterpolationType.at(
							model_opt_ptree.get<std::string>("acceptance_interpolation")));
			if (!model_opt_ptree.get<bool>(
					"automatic_acceptance_boundary_finding_active")) {
				data_model->setAcceptanceBounds(
						model_opt_ptree.get<double>("acceptance_bound_low"),
						model_opt_ptree.get<double>("acceptance_bound_high"));
			}
			data_model->setDataDimension(
					acceptance.getPrimaryDimension().dimension_range);
			shared_ptr<Model1D> acc(data_model);

			current_model.reset(
					new ProductModel1D("acceptance_corrected_1d", current_model, acc));

			gPad = current_pad; // reset pad to the one before
		} else {
			std::cout
					<< "ERROR: requesting an acceptance corrected model without an acceptance object!"
					<< std::endl;
		}
	}

	if (model_opt_ptree.get<bool>("resolution_smearing_active")) { // with resolution smearing
		// ok since we have smearing on, generate smearing model

	}

// every model has superior parameters which have to be set by the user
// if they are kept fixed
// in this case its the lab momentum plab
	current_model->getModelParameterSet().setModelParameterValue("p_lab",
			data.getLabMomentum());
// the luminosity (we set it to 1.0 as a default value)
	current_model->getModelParameterSet().setModelParameterValue("luminosity",
			1.0);
	if (current_model->init()) {
		std::cout
				<< "ERROR: Not all parameters of the model were successfully initialized!"
				<< std::endl;
		current_model->getModelParameterSet().printInfo();
	}

	return current_model;
}

shared_ptr<Model2D> PndLmdModelFactory::generate2DModel(
		const boost::property_tree::ptree& model_opt_ptree,
		const PndLmdAngularData& data) const {
	shared_ptr<Model2D> current_model;

	std::stringstream model_name;
	model_name << "dpm_angular_2d";

	shared_ptr<PndLmdDPMAngModel1D> dpm_angular_1d(
			new PndLmdDPMAngModel1D("dpm_angular_1d",
					LumiFit::StringToDPMElasticParts.at(
							model_opt_ptree.get<std::string>("dpm_elastic_parts"))));

	shared_ptr<Parametrization> dpm_parametrization(
			new PndLmdDPMModelParametrization(
					dpm_angular_1d->getModelParameterSet()));
	dpm_angular_1d->getModelParameterHandler().registerParametrizations(
			dpm_angular_1d->getModelParameterSet(), dpm_parametrization);

	const LumiFit::LmdDimension& data_primary_dimension =
			data.getPrimaryDimension();
	const LumiFit::LmdDimension& data_secondary_dimension =
			data.getSecondaryDimension();

	if (data_primary_dimension.dimension_options.dimension_type
			== LumiFit::THETA_X
			&& data_secondary_dimension.dimension_options.dimension_type
					== LumiFit::THETA_Y) {

		if (model_opt_ptree.get<bool>("divergence_smearing_active")
				&& data_primary_dimension.is_active
				&& data_secondary_dimension.is_active) {
			// add the divergence smearing on top
			// at this point we introduce "non numerical" discretization from the data
			// thats why the data object is required here

			// make the binning twice as fine
			LumiFit::LmdDimension temp_prim_dim(data.getPrimaryDimension());
			LumiFit::LmdDimension temp_sec_dim(data.getSecondaryDimension());

		//	temp_prim_dim.bins = temp_prim_dim.bins * 4;
		//	temp_sec_dim.bins = temp_sec_dim.bins * 4;
		//	temp_prim_dim.calculateBinSize();
		//	temp_sec_dim.calculateBinSize();

			shared_ptr<PndLmdFastDPMAngModel2D> fast_dpm_model_2d(
					new PndLmdFastDPMAngModel2D(model_name.str(), dpm_angular_1d,
							temp_prim_dim, temp_sec_dim,
							PndLmdRuntimeConfiguration::Instance().getNumberOfThreads()));

			shared_ptr<GaussianModel2D> divergence_model(
					new GaussianModel2D("divergence_model", 4.0));

			divergence_model->init();

			shared_ptr<PndLmdDivergenceSmearingModel2D> divergence_smearing_model(
					new PndLmdDivergenceSmearingModel2D(divergence_model, temp_prim_dim,
							temp_sec_dim));

			model_name << "_div_smeared";

			shared_ptr<PndLmdDifferentialSmearingConvolutionModel2D> div_smeared_model(
					new PndLmdDifferentialSmearingConvolutionModel2D(model_name.str(),
							fast_dpm_model_2d, divergence_smearing_model, data_primary_dimension,
							data_secondary_dimension,
							PndLmdRuntimeConfiguration::Instance().getNumberOfThreads()));
			div_smeared_model->injectModelParameter(
					divergence_model->getModelParameterSet().getModelParameter(
							"gauss_sigma_var1"));
			div_smeared_model->injectModelParameter(
					divergence_model->getModelParameterSet().getModelParameter(
							"gauss_sigma_var2"));

			current_model = div_smeared_model;
		} else {
			current_model.reset(
					new PndLmdFastDPMAngModel2D(model_name.str(), dpm_angular_1d,
							data_primary_dimension, data_secondary_dimension,
							PndLmdRuntimeConfiguration::Instance().getNumberOfThreads()));
		}
	} else {
		if (model_opt_ptree.get<bool>("divergence_smearing_active")
				&& data_primary_dimension.is_active
				&& data_secondary_dimension.is_active) {
			std::cout
					<< "WARNING: Divergence smearing is not available in this coordinate system!<<"
							" Please use the theta_xy frame." << std::endl;
		}
		current_model.reset(
				new PndLmdDPMAngModel2D(model_name.str(), dpm_angular_1d));
	}

	if (model_opt_ptree.get<bool>("acceptance_correction_active")) { // with acceptance corr
		if (acceptance.getAcceptance2D()) {
			shared_ptr<DataModel2D> acc(
					new DataModel2D("acceptance_2d",
							LumiFit::StringToInterpolationType.at(
									model_opt_ptree.get<std::string>(
											"acceptance_interpolation"))));

			TVirtualPad* curpad = gPad;
			TCanvas c;
			acceptance.getAcceptance2D()->Draw("colz");
			c.Update();
			TH2 *hist = acceptance.getAcceptance2D()->GetPaintedHistogram();

			std::pair<double, double> pos;
			std::map<std::pair<double, double>, double> datamap;
			for (unsigned int ix = 0; ix < data_primary_dimension.bins; ix++) {
				for (unsigned int iy = 0; iy < data_secondary_dimension.bins; iy++) {
					pos.first = data_primary_dimension.dimension_range.getRangeLow()
							+ (0.5 + ix) * data_primary_dimension.bin_size;
					pos.second = data_secondary_dimension.dimension_range.getRangeLow()
							+ (0.5 + iy) * data_secondary_dimension.bin_size;
					datamap[pos] = hist->Interpolate(pos.first, pos.second);
					/*std::cout << pos.first << ", " << pos.second << ": "
					 << hist->Interpolate(pos.first, pos.second) << std::endl;*/
				}
			}

			gPad = curpad;

			acc->setData(datamap);

			model_name << "_acceptance_corrected";

			current_model.reset(
					new ProductModel2D(model_name.str(), current_model, acc));

			/*current_model->getModelParameterSet().freeModelParameter("offset_x");
			 current_model->getModelParameterSet().freeModelParameter("offset_y");
			 if (model_opt_ptree.get<bool>("fix_ip_offsets")) {
			 current_model->getModelParameterSet().getModelParameter("offset_x")->setValue(
			 model_opt_ptree.get<double>("ip_offset_x"));
			 current_model->getModelParameterSet().getModelParameter("offset_x")->setParameterFixed(
			 true);
			 current_model->getModelParameterSet().getModelParameter("offset_y")->setValue(
			 model_opt_ptree.get<double>("ip_offset_y"));
			 current_model->getModelParameterSet().getModelParameter("offset_y")->setParameterFixed(
			 true);
			 }*/
		} else {
			std::cout
					<< "ERROR: requesting an acceptance corrected model without an acceptance object!"
					<< std::endl;
		}
	}

	if (model_opt_ptree.get<bool>("resolution_smearing_active")) { // with resolution smearing
		// ok since we have smearing on, generate smearing model

		model_name << "_res_smeared";

		current_model.reset(
				new PndLmdSmearingConvolutionModel2D(model_name.str(), current_model,
						generate2DSmearingModel(data)));
	}

	// every model has superior parameters which have to be set by the user
	// if they are kept fixed
	// in this case its the lab momentum plab
	dpm_angular_1d->getModelParameterSet().setModelParameterValue("p_lab",
			data.getLabMomentum());
	// free the luminosity (we set it to 1.0 as a default value)
	/*dpm_angular_1d->getModelParameterSet().freeModelParameter("luminosity");
	 dpm_angular_1d->getModelParameterSet().setModelParameterValue("luminosity",
	 1.0);*/

	current_model->init();

	return current_model;
}

void PndLmdModelFactory::setAcceptance(const PndLmdAcceptance& acceptance_) {
	acceptance = acceptance_;
}

void PndLmdModelFactory::setResolutions(
		const std::vector<PndLmdHistogramData>& resolutions_) {
	resolutions = resolutions_;
}

shared_ptr<Model1D> PndLmdModelFactory::generate1DVertexModel(
		const boost::property_tree::ptree& model_opt_ptree) const {
	shared_ptr<Model1D> vertex_model;

	LumiFit::ModelType model_type(
			LumiFit::StringToModelType.at(
					model_opt_ptree.get<std::string>("model_type")));

	if (model_type == LumiFit::UNIFORM) {
		vertex_model.reset(new DoubleGaussianModel1D("vertex_double_gaussian_1d"));
	} else if (model_type == LumiFit::GAUSSIAN) {
		vertex_model.reset(new GaussianModel1D("vertex_gaussian_1d"));
	} else {
		std::cout
				<< "Unknown vertex model! Only uniform and gaussian model is allowed."
				<< std::endl;
	}

	return vertex_model;
}

shared_ptr<Model> PndLmdModelFactory::generateModel(
		const boost::property_tree::ptree& model_opt_ptree,
		const PndLmdAngularData& data) const {
	shared_ptr<Model> current_model;

	std::cout << "Generating model ... " << std::endl;

	unsigned int fit_dimension = model_opt_ptree.get<unsigned int>(
			"fit_dimension");
	if (fit_dimension == 1) {
		current_model = generate1DModel(model_opt_ptree, data);
	} else if (fit_dimension == 2) {
		current_model = generate2DModel(model_opt_ptree, data);
	} else {
		std::cout << "WARNING: Requesting a model of dimension " << fit_dimension
				<< " which is NOT available! Returning NULL pointer." << std::endl;
	}
	return current_model;
}
