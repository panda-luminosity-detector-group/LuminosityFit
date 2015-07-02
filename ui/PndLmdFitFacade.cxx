#include "PndLmdFitFacade.h"
#include "data/PndLmdAngularData.h"
#include "fit/estimatorImpl/Chi2Estimator.h"
#include "fit/estimatorImpl/LogLikelihoodEstimator.h"
#include "fit/minimizerImpl/ROOT/ROOTMinimizer.h"
#include "fit/data/Data.h"
#include "PndLmdDataFacade.h"
#include "PndLmdComparisonStructs.h"
#include "model/PndLmdModelFactory.h"

#include <iostream>
#include <algorithm>

#include "boost/property_tree/ptree.hpp"
#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"

#include "TFile.h"
#include "TH1D.h"

using std::cout;
using std::endl;
using std::vector;
using std::pair;

using boost::property_tree::ptree;

PndLmdFitFacade::PndLmdFitFacade() :
		lmd_runtime_config(PndLmdRuntimeConfiguration::Instance()) {
}

PndLmdFitFacade::~PndLmdFitFacade() {
}

void PndLmdFitFacade::addAcceptencesToPool(
		const std::vector<PndLmdAcceptance> &lmd_acc) {
	acceptance_pool.insert(lmd_acc.begin(), lmd_acc.end());
}
void PndLmdFitFacade::addResolutionsToPool(
		const std::vector<PndLmdHistogramData> &lmd_res) {
	resolution_pool.insert(lmd_res.begin(), lmd_res.end());
}

void PndLmdFitFacade::clearPools() {
	acceptance_pool.clear();
	resolution_pool.clear();
}

std::vector<DataStructs::DimensionRange> PndLmdFitFacade::calcRange(
		const PndLmdAbstractData &lmd_abs_data,
		const EstimatorOptions &est_options) const {
	DataStructs::DimensionRange temp_range;
	std::vector<DataStructs::DimensionRange> ranges;
	temp_range.range_low =
			lmd_abs_data.getPrimaryDimension().dimension_range.getRangeLow();
	temp_range.range_high =
			lmd_abs_data.getPrimaryDimension().dimension_range.getRangeHigh();
	if (est_options.getFitRangeX().is_active) {
		if (est_options.getFitRangeX().range_low
				> lmd_abs_data.getPrimaryDimension().dimension_range.getRangeLow())
			temp_range.range_low = est_options.getFitRangeX().range_low;
		if (est_options.getFitRangeX().range_high
				< lmd_abs_data.getPrimaryDimension().dimension_range.getRangeHigh())
			temp_range.range_high = est_options.getFitRangeX().range_high;
	}
	ranges.push_back(temp_range);

	if (lmd_abs_data.getSecondaryDimension().is_active) {
		temp_range.range_low =
				lmd_abs_data.getSecondaryDimension().dimension_range.getRangeLow();
		temp_range.range_high =
				lmd_abs_data.getSecondaryDimension().dimension_range.getRangeHigh();

		if (est_options.getFitRangeY().is_active) {
			if (est_options.getFitRangeY().range_low
					> lmd_abs_data.getSecondaryDimension().dimension_range.getRangeLow())
				temp_range.range_low = est_options.getFitRangeY().range_low;
			if (est_options.getFitRangeY().range_high
					< lmd_abs_data.getSecondaryDimension().dimension_range.getRangeHigh())
				temp_range.range_high = est_options.getFitRangeY().range_high;

			std::cout << "fit range was active and changed the range to "
					<< temp_range.range_low << "-" << temp_range.range_high << std::endl;
		}

		ranges.push_back(temp_range);
	}

	return ranges;
}

double PndLmdFitFacade::calcHistIntegral(const TH1D* hist,
		std::vector<DataStructs::DimensionRange> range) const {
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

double PndLmdFitFacade::calcHistIntegral(const TH2D* hist,
		std::vector<DataStructs::DimensionRange> range) const {
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

shared_ptr<Data> PndLmdFitFacade::createData1D(
		const PndLmdHistogramData &lmd_hist_data) const {
	shared_ptr<Data> data(new Data(1));
	data_helper.fillBinnedData(data, lmd_hist_data.get1DHistogram());
	return data;
}
shared_ptr<Data> PndLmdFitFacade::createData2D(
		const PndLmdHistogramData &lmd_hist_data) const {
	shared_ptr<Data> data(new Data(2));
	data_helper.fillBinnedData(data, lmd_hist_data.get2DHistogram());
	return data;
}

void PndLmdFitFacade::saveFittedObjectsToFile(
		std::vector<PndLmdAngularData>& lmd_data_vec) const {
	for (unsigned int i = 0; i < lmd_data_vec.size(); ++i) {
		if (lmd_data_vec[i].getFitResults().size() > 0) {
			lmd_data_vec[i].saveToRootFile();
		}
	}
}

// estimator options

EstimatorOptions PndLmdFitFacade::constructEstimatorOptionsFromConfig(
		const ptree& pt) const {
	EstimatorOptions est_opt;

	est_opt.setWithIntegralScaling(pt.get<bool>("with_integral_scaling"));

	DataStructs::DimensionRange dim_range;
	dim_range.is_active = pt.get<bool>("fit_range_x_active");
	dim_range.range_low = pt.get<double>("fit_range_x_low");
	dim_range.range_high = pt.get<double>("fit_range_x_high");
	est_opt.setFitRangeX(dim_range);

	dim_range.is_active = pt.get<bool>("fit_range_y_active");
	dim_range.range_low = pt.get<double>("fit_range_y_low");
	dim_range.range_high = pt.get<double>("fit_range_y_high");
	est_opt.setFitRangeY(dim_range);

	return est_opt;
}

std::set<std::string, ModelStructs::string_comp> PndLmdFitFacade::constructFreeFitParameterListFromConfig(
		const ptree& pt) const {
	std::set<std::string, ModelStructs::string_comp> free_params;
	BOOST_FOREACH(const ptree::value_type & free_param_name, pt){
	free_params.insert(free_param_name.first);
}
	return free_params;
}

void PndLmdFitFacade::freeParametersForModel(shared_ptr<Model> current_model,
		const PndLmdFitOptions &fit_opts) const {
	const std::set<std::string, ModelStructs::string_comp> &free_params =
			fit_opts.getFreeParameterSet();

	std::set<std::string, ModelStructs::string_comp>::const_iterator it;
	for (it = free_params.begin(); it != free_params.end(); ++it) {
		current_model->getModelParameterSet().freeModelParameter(*it);
	}
}

void PndLmdFitFacade::initBeamParametersForModel(
		shared_ptr<Model> current_model, const ptree& model_opt_ptree) const {
	current_model->getModelParameterSet().setModelParameterValue("luminosity",
			1.0);

	current_model->getModelParameterSet().getModelParameter("tilt_x")->setValue(
			model_opt_ptree.get<double>("beam_tilt_x"));
	current_model->getModelParameterSet().getModelParameter("tilt_y")->setValue(
			model_opt_ptree.get<double>("beam_tilt_y"));

	current_model->getModelParameterSet().getModelParameter("offset_x")->setValue(
			model_opt_ptree.get<double>("ip_offset_x"));
	current_model->getModelParameterSet().getModelParameter("offset_y")->setValue(
			model_opt_ptree.get<double>("ip_offset_y"));

	if (model_opt_ptree.get<bool>("divergence_smearing_active")) {
		current_model->getModelParameterSet().getModelParameter("gauss_sigma_var1")->setValue(
				model_opt_ptree.get<double>("beam_div_x"));
		current_model->getModelParameterSet().getModelParameter("gauss_sigma_var2")->setValue(
				model_opt_ptree.get<double>("beam_div_y"));
		/*current_model->getModelParameterSet().getModelParameter("gauss_mean_var1")->setValue(
		 0.0);
		 current_model->getModelParameterSet().getModelParameter("gauss_mean_var2")->setValue(
		 0.0);
		 current_model->getModelParameterSet().getModelParameter("gauss_rho")->setValue(
		 0.0);*/
	}
}

void PndLmdFitFacade::addBeamParametersToFreeParameterList(
		PndLmdFitOptions &fit_opts, const ptree &model_opt_ptree) const {
	if (!model_opt_ptree.get<bool>("fix_beam_tilts")) {
		fit_opts.free_parameter_names.insert("tilt_x");
		fit_opts.free_parameter_names.insert("tilt_y");
	}

	if (!model_opt_ptree.get<bool>("fix_beam_divs")
			&& model_opt_ptree.get<bool>("divergence_smearing_active")) {
		fit_opts.free_parameter_names.insert("gauss_sigma_var1");
		fit_opts.free_parameter_names.insert("gauss_sigma_var2");
	}

	if (!model_opt_ptree.get<bool>("fix_ip_offsets")) {
		fit_opts.free_parameter_names.insert("offset_x");
		fit_opts.free_parameter_names.insert("offset_y");
	}

	fit_opts.free_parameter_names.insert("luminosity");
}

PndLmdFitOptions PndLmdFitFacade::createFitOptions(
		const PndLmdAbstractData &lmd_data) const {
	std::cout << "creating fit options..." << std::endl;

	const ptree& fit_config_ptree = lmd_runtime_config.getFitConfigTree();

	PndLmdFitOptions fit_opts;

	fit_opts.estimator_type = LumiFit::StringToLmdEstimatorType.at(
			fit_config_ptree.get<std::string>("fit.estimator_type"));
	fit_opts.est_opt = constructEstimatorOptionsFromConfig(
			fit_config_ptree.get_child("fit.estimator_options"));
	ptree model_option_ptree = fit_config_ptree.get_child(
			"fit.fit_model_options");
	fit_opts.free_parameter_names = constructFreeFitParameterListFromConfig(
			fit_config_ptree.get_child("fit.free_parameter_names"));

// switch range to momentum transfer as user usually specifies that in mrad
	if (LumiFit::T
			== lmd_data.getPrimaryDimension().dimension_options.dimension_type) {
		model_option_ptree.put("momentum_transfer_active", true);
		model_option_ptree.put("acceptance_correction_active", false);
		model_option_ptree.put("resolution_smearing_active", false);

		PndLmdModelFactory model_factory;
		// recalcuated ranges to momentum transfer
		DataStructs::DimensionRange temp_range;
		temp_range.range_low = model_factory.getMomentumTransferFromTheta(
				lmd_data.getLabMomentum(),
				fit_opts.getEstimatorOptions().getFitRangeX().range_low);
		temp_range.range_high = model_factory.getMomentumTransferFromTheta(
				lmd_data.getLabMomentum(),
				fit_opts.getEstimatorOptions().getFitRangeX().range_high);

		fit_opts.est_opt.setFitRangeX(temp_range);
	} else if (lmd_data.getPrimaryDimension().dimension_options.dimension_type
			== LumiFit::THETA
			|| lmd_data.getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::THETA_X) {
		addBeamParametersToFreeParameterList(fit_opts, model_option_ptree);

		model_option_ptree.put("momentum_transfer_active", false);
		model_option_ptree.put("acceptance_correction_active", false);
		model_option_ptree.put("resolution_smearing_active", false);
		if (LumiFit::MC_ACC
				== lmd_data.getPrimaryDimension().dimension_options.track_type)
			model_option_ptree.put("acceptance_correction_active", true);
		if (LumiFit::RECO
				== lmd_data.getPrimaryDimension().dimension_options.track_type) {
			model_option_ptree.put("acceptance_correction_active", true);
			model_option_ptree.put("resolution_smearing_active", true);
		}
	}

	ptree::iterator iter;
	// convert the ptree to simple format...
	for (iter = model_option_ptree.begin(); iter != model_option_ptree.end();
			iter++) {
		fit_opts.model_opt_map[iter->first] = iter->second.data();
	}

	return fit_opts;
}

shared_ptr<Model> PndLmdFitFacade::generateModel(
		const PndLmdAngularData &lmd_data,
		const PndLmdFitOptions &fit_options) const {
	// create a new model via the factory
	shared_ptr<Model> model = model_factory.generateModel(
			fit_options.getModelOptionsPropertyTree(), lmd_data);

	// free parameters
	if (model.get()) {
		//always free luminosity
		std::cout << "freeing model parameters..." << std::endl;
		model->getModelParameterSet().freeModelParameter("luminosity");

		//loop over all other free parameter names
		std::set<std::string, ModelStructs::string_comp> free_parameter_names =
				fit_options.getFreeParameterSet();

		for (std::set<std::string, ModelStructs::string_comp>::iterator it =
				free_parameter_names.begin(); it != free_parameter_names.end(); it++) {
			model->getModelParameterSet().freeModelParameter(*it);
		}
	}

	return model;
}

void PndLmdFitFacade::doFit(PndLmdHistogramData &lmd_hist_data,
		const PndLmdFitOptions &fit_options) {

	cout << "Attempting to perform fit with following fit options:" << endl;
	cout << fit_options << endl;

//first check if this model with the fit options have already been fitted
	ModelFitResult fit_result = lmd_hist_data.getFitResult(fit_options);
	if (fit_result.getFitParameters().size() > 0) {
		cout << "Fit was already performed! Skipping..." << endl;
		return;
	}

	// create estimator
	shared_ptr<ModelEstimator> estimator;
	if (fit_options.estimator_type == LumiFit::CHI2)
		estimator.reset(new Chi2Estimator());
	else
		estimator.reset(new LogLikelihoodEstimator());

	PndLmdRuntimeConfiguration& lmd_runtime_config =
			PndLmdRuntimeConfiguration::Instance();

	estimator->setNumberOfThreads(lmd_runtime_config.getNumberOfThreads());

	model_fit_facade.setEstimator(estimator);

	model_fit_facade.setEstimatorOptions(fit_options.getEstimatorOptions());

	fit_result = model_fit_facade.Fit();

// store fit results
	cout << "Adding fit result to storage..." << endl;

	lmd_hist_data.addFitResult(fit_options, fit_result);
	cout << "fit storage now contains " << lmd_hist_data.getFitResults().size()
			<< " entries!" << endl;
}

PndLmdFitDataBundle PndLmdFitFacade::doLuminosityFits(
		std::vector<PndLmdAngularData>& lmd_data_vec) {
	cout << "Running LumiFit...." << endl;

	PndLmdFitDataBundle data_bundle;

	PndLmdDataFacade lmd_data_facade;

	std::vector<PndLmdAcceptance> matching_acc;
	std::vector<PndLmdHistogramData> matching_res;

	std::vector<PndLmdAcceptance> empty_acc_vec;
	std::vector<PndLmdHistogramData> empty_res_vec;

	for (unsigned int elastic_data_index = 0;
			elastic_data_index < lmd_data_vec.size(); ++elastic_data_index) {
		PndLmdFitOptions fit_options(
				createFitOptions(lmd_data_vec[elastic_data_index]));

		std::vector<PndLmdAcceptance> *acc_vec_link = &empty_acc_vec;
		std::vector<PndLmdHistogramData> *res_vec_link = &empty_res_vec;

		if (fit_options.getModelOptionsPropertyTree().get<bool>(
				"resolution_smearing_active")) {
			matching_res = lmd_data_facade.getMatchingResolutions(resolution_pool,
					lmd_data_vec[elastic_data_index]);
			model_factory.setResolutions(matching_res);
			res_vec_link = &matching_res;
		}

		if (fit_options.getModelOptionsPropertyTree().get<bool>(
				"acceptance_correction_active")) {
			matching_acc = lmd_data_facade.getMatchingAcceptances(acceptance_pool,
					lmd_data_vec[elastic_data_index]);

			for (unsigned int acc_index = 0; acc_index < matching_acc.size();
					++acc_index) {
				// set acc in factory
				model_factory.setAcceptance(matching_acc[acc_index]);
				fitElasticPPbar(lmd_data_vec[elastic_data_index], fit_options);
			}

			acc_vec_link = &matching_acc;

		} else {
			fitElasticPPbar(lmd_data_vec[elastic_data_index], fit_options);
		}

		data_bundle.addFittedElasticData(lmd_data_vec[elastic_data_index],
				*acc_vec_link, *res_vec_link);
	}

	return data_bundle;
}

void PndLmdFitFacade::fitElasticPPbar(PndLmdAngularData &lmd_data,
		const PndLmdFitOptions &fit_options) {
	// generate model

	shared_ptr<Model> model = model_factory.generateModel(
			fit_options.getModelOptionsPropertyTree(), lmd_data);

	// init beam parameters in model
	initBeamParametersForModel(model, fit_options.getModelOptionsPropertyTree());

	if (model->init()) {
		std::cout
				<< "ERROR: Not all parameters of the model were successfully initialized!"
				<< std::endl;
		model->getModelParameterSet().printInfo();
	}

	// free parameters
	freeParametersForModel(model, fit_options);

	// set model
	model_fit_facade.setModel(model);

	unsigned int fit_dimension = fit_options.getModelOptionsPropertyTree().get<
			unsigned int>("fit_dimension");

	// create and set data
	if (fit_dimension == 2) {
		std::cout << "creating 2D data..." << std::endl;
		model_fit_facade.setData(createData2D(lmd_data));
	} else {
		std::cout << "creating 1D data..." << std::endl;
		model_fit_facade.setData(createData1D(lmd_data));
	}

	// now set better starting amplitude value
	std::vector<DataStructs::DimensionRange> range = calcRange(lmd_data,
			fit_options.getEstimatorOptions());
	double integral_data = 0.0;
	std::cout << "calculating data integral..." << std::endl;
	if (fit_dimension == 2)
		integral_data = calcHistIntegral(lmd_data.get2DHistogram(),
				range);
	else
		integral_data = calcHistIntegral(lmd_data.get1DHistogram(),
				range);
	std::cout << "calculating model integral..." << std::endl;
	double integral_func = model->Integral(range, 1e-1);
	double binning_factor = lmd_data.getBinningFactor(fit_dimension);
	double lumi_start = integral_data / integral_func / binning_factor;
	cout << "binning factor: " << binning_factor << endl;
	cout << integral_data << " / " << integral_func * binning_factor << endl;
	cout << "Using start luminosity: " << lumi_start << endl;
	model->getModelParameterSet().setModelParameterValue("luminosity",
			lumi_start);

	// create minimizer instance with control parameter
	shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());

	model_fit_facade.setMinimizer(minuit_minimizer);

	doFit(lmd_data, fit_options);
}

void PndLmdFitFacade::fitVertexData(
		std::vector<PndLmdHistogramData> &lmd_data) {
	for (unsigned int i = 0; i < lmd_data.size(); i++) {
		cout << "Fitting resolution " << lmd_data[i].getName() << endl;

		PndLmdFitOptions fit_options(createFitOptions(lmd_data[i]));

		// get histogram
		const TH1D* hist = lmd_data[i].get1DHistogram();
		if (hist->Integral() < 400) {
			return;
		}

		ptree sim_params(lmd_data[i].getSimulationParametersPropertyTree());

		DataStructs::DimensionRange old_range = fit_options.est_opt.getFitRangeX();
		double ideal_sigma;
		if (lmd_data[i].getPrimaryDimension().dimension_options.track_type
				== LumiFit::MC) {
			DataStructs::DimensionRange fit_range;

			if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::X) {
				ideal_sigma = sim_params.get<double>("ip_standard_deviation_x");
				fit_range.range_low = sim_params.get<double>("ip_mean_x")
						- 2.0 * ideal_sigma;
				fit_range.range_high = sim_params.get<double>("ip_mean_x")
						+ 2.0 * ideal_sigma;
			} else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::Y) {
				ideal_sigma = sim_params.get<double>("ip_standard_deviation_y");
				fit_range.range_low = sim_params.get<double>("ip_mean_y")
						- 2.0 * ideal_sigma;
				fit_range.range_high = sim_params.get<double>("ip_mean_y")
						+ 2.0 * ideal_sigma;
			} else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::Z) {
				ideal_sigma = sim_params.get<double>("ip_standard_deviation_z");
				fit_range.range_low = sim_params.get<double>("ip_mean_z")
						- 2.0 * ideal_sigma;
				fit_range.range_high = sim_params.get<double>("ip_mean_z")
						+ 2.0 * ideal_sigma;
			}

			fit_range.is_active = true;
			fit_options.est_opt.setFitRangeX(fit_range);
		} else if (lmd_data[i].getPrimaryDimension().dimension_options.track_type
				== LumiFit::RECO) {
			DataStructs::DimensionRange fit_range;

			if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::X) {
				ideal_sigma = hist->GetRMS(1);
				fit_range.range_low = sim_params.get<double>("ip_mean_x")
						- 2.0 * ideal_sigma;
				fit_range.range_high = sim_params.get<double>("ip_mean_x")
						+ 2.0 * ideal_sigma;
			} else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::Y) {
				ideal_sigma = hist->GetRMS(1);
				fit_range.range_low = sim_params.get<double>("ip_mean_y")
						- 2.0 * ideal_sigma;
				fit_range.range_high = sim_params.get<double>("ip_mean_y")
						+ 2.0 * ideal_sigma;
			} else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
					== LumiFit::Z) {
				ideal_sigma = hist->GetRMS(1);
				fit_range.range_low = sim_params.get<double>("ip_mean_z")
						- 2.0 * ideal_sigma;
				fit_range.range_high = sim_params.get<double>("ip_mean_z")
						+ 2.0 * ideal_sigma;
			}

			fit_range.is_active = true;
			fit_options.est_opt.setFitRangeX(fit_range);
		}

		// create chi2 estimator
		shared_ptr<Chi2Estimator> chi2_est(new Chi2Estimator());
		model_fit_facade.setEstimator(chi2_est);

		/*shared_ptr<LogLikelihoodEstimator> loglikelihood_est(
		 new LogLikelihoodEstimator());
		 model_fit_facade.setEstimator(loglikelihood_est);*/

		model_fit_facade.setData(createData1D(lmd_data[i]));

		ptree model_opt_ptree = fit_options.getModelOptionsPropertyTree();
		// generate the model
		shared_ptr<Model1D> vertex_model = model_factory.generate1DVertexModel(
				model_opt_ptree);

		LumiFit::ModelType model_type = LumiFit::StringToModelType.at(
				model_opt_ptree.get<std::string>("model_type"));

		// now we have to set good starting values and free parameters
		if (model_type == LumiFit::GAUSSIAN) { // simple gaussian
			// amplitude of gauss is equal to number of events in the histogram
			vertex_model->getModelParameterSet().getModelParameter("gauss_amplitude")->setParameterFixed(
					false);
			vertex_model->getModelParameterSet().getModelParameter("gauss_amplitude")->setValue(
					hist->Integral());
			vertex_model->getModelParameterSet().getModelParameter("gauss_mean")->setValue(
					hist->GetMean(1));
			vertex_model->getModelParameterSet().getModelParameter("gauss_mean")->setParameterFixed(
					false);
			if (lmd_data[i].getPrimaryDimension().dimension_options.track_type
					== LumiFit::MC) {
				vertex_model->getModelParameterSet().getModelParameter("gauss_sigma")->setValue(
						ideal_sigma);
			} else {
				vertex_model->getModelParameterSet().getModelParameter("gauss_sigma")->setValue(
						hist->GetRMS(1));
			}
			vertex_model->getModelParameterSet().getModelParameter("gauss_sigma")->setParameterFixed(
					false);
		}

		// set model
		model_fit_facade.setModel(vertex_model);

		// create minimizer instance with control parameter
		shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());

		model_fit_facade.setMinimizer(minuit_minimizer);

		doFit(lmd_data[i], fit_options);
	}
}
