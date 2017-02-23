#include "PndLmdFitFacade.h"
#include "data/PndLmdAngularData.h"
#include "fit/estimatorImpl/Chi2Estimator.h"
#include "fit/estimatorImpl/LogLikelihoodEstimator.h"
#include "fit/minimizerImpl/ROOT/ROOTMinimizer.h"
//#include "fit/minimizerImpl/Ceres/CeresMinimizer.h"
#include "fit/data/Data.h"
#include "PndLmdDataFacade.h"
#include "PndLmdComparisonStructs.h"
#include "model/PndLmdModelFactory.h"

#include <iostream>
#include <algorithm>
#include <csignal>

#include "boost/property_tree/ptree.hpp"
#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"

#include "TFile.h"
#include "TH1D.h"
#include "TVectorD.h"

#include "callgrind.h"

using std::cout;
using std::endl;
using std::vector;
using std::pair;

using boost::property_tree::ptree;

PndLmdFitFacade::PndLmdFitFacade() :
    lmd_runtime_config(PndLmdRuntimeConfiguration::Instance()) {
  signal(SIGINT, signalHandler);
}

PndLmdFitFacade::~PndLmdFitFacade() {
}

void PndLmdFitFacade::signalHandler(int signum) {
  cout << "Interrupt signal (" << signum << ") received.\n";

  CALLGRIND_STOP_INSTRUMENTATION;
  CALLGRIND_DUMP_STATS;

  // cleanup and close up stuff here
  // terminate program
  exit(signum);
}

void PndLmdFitFacade::setModelFactoryAcceptence(
    const PndLmdAcceptance &lmd_acc) {
  model_factory.setAcceptance(lmd_acc);
}
void PndLmdFitFacade::setModelFactoryResolutions(
    const std::vector<PndLmdHistogramData> &lmd_res) {
  model_factory.setResolutions(lmd_res);
}
void PndLmdFitFacade::setModelFactoryResolutionMap(
    const PndLmdMapData &lmd_res) {
  model_factory.setResolutionMapData(lmd_res);
}

void PndLmdFitFacade::addAcceptencesToPool(
    const std::vector<PndLmdAcceptance> &lmd_acc) {
  acceptance_pool.insert(acceptance_pool.end(), lmd_acc.begin(), lmd_acc.end());
}
void PndLmdFitFacade::addResolutionsToPool(
    const std::vector<PndLmdHistogramData> &lmd_res) {
  resolution_pool.insert(lmd_res.begin(), lmd_res.end());
}
void PndLmdFitFacade::addResolutionMapsToPool(
    const std::vector<PndLmdMapData> &lmd_res) {
  resolution_map_pool.insert(lmd_res.begin(), lmd_res.end());
}

void PndLmdFitFacade::clearPools() {
  acceptance_pool.clear();
  resolution_pool.clear();
  resolution_map_pool.clear();
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

  /*current_model->getModelParameterSet().getModelParameter("offset_x")->setValue(
      model_opt_ptree.get<double>("ip_offset_x"));
  current_model->getModelParameterSet().getModelParameter("offset_y")->setValue(
      model_opt_ptree.get<double>("ip_offset_y"));*/

  if (model_opt_ptree.get<bool>("divergence_smearing_active")) {
    double start_div_x(0.0001);
    double start_div_y(0.0001);
    // bool optional
    boost::optional<double> v1 = model_opt_ptree.get_optional<double>(
        "beam_div_x");
    boost::optional<double> v2 = model_opt_ptree.get_optional<double>(
        "beam_div_y");

    if (v1) {
      start_div_x = v1.get();
    } else {
      // if parameters are not set we use the simulated ones
      if (!PndLmdRuntimeConfiguration::Instance().getSimulationParameters().empty())
        start_div_x =
            PndLmdRuntimeConfiguration::Instance().getSimulationParameters().get<
                double>("beam_divergence_x");
    }

    if (v2) {
      start_div_y = v2.get();
    } else {
      // if parameters are not set we use the simulated ones
      if (!PndLmdRuntimeConfiguration::Instance().getSimulationParameters().empty())
        start_div_y =
            PndLmdRuntimeConfiguration::Instance().getSimulationParameters().get<
                double>("beam_divergence_y");
    }

    current_model->getModelParameterSet().getModelParameter(
              "gauss_sigma_var1")->setValue(start_div_x);
    current_model->getModelParameterSet().getModelParameter(
              "gauss_sigma_var2")->setValue(start_div_y);

    std::cout<<"using start divergence parameters: \n";
    std::cout<<"div_x: "<<start_div_x<<std::endl;
    std::cout<<"div_y: "<<start_div_y<<std::endl;
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
        == lmd_data.getPrimaryDimension().dimension_options.track_type) {
      model_option_ptree.put("acceptance_correction_active", true);
    }
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

PndLmdFitDataBundle PndLmdFitFacade::doLuminosityFits(
    std::vector<PndLmdAngularData>& lmd_data_vec) {
  PndLmdDataFacade lmd_data_facade;
  PndLmdFitDataBundle data_bundle;

  std::vector<PndLmdAcceptance> matching_acc;

  cout << "Running LumiFit on " << lmd_data_vec.size()
      << " angular data sets...." << endl;

  for (auto& lmd_data : lmd_data_vec) {

    PndLmdFitOptions fit_options(createFitOptions(lmd_data));

    if (fit_options.getModelOptionsPropertyTree().get<bool>(
        "resolution_smearing_active")) {
      //matching_res = lmd_data_facade.getMatchingResolutions(resolution_pool,
      //    lmd_data_vec[elastic_data_index]);
      //model_factory.setResolutions(matching_res);
      if (resolution_map_pool.size() > 0) {
        if (resolution_map_pool.begin()->getHitMap().size() == 0) {
          std::cout
              << "Requesting fit with resolution smearing, however resolution map data is empty!"
              << "Hence skipping this fit!\n";
          continue;
        }
        model_factory.setResolutionMapData(*resolution_map_pool.begin());
      } else {
        std::cout
            << "Requesting fit with resolution smearing, however no resolution map data was specified!"
            << "Hence skipping this fit!\n";
        continue;
      }
    }

    if (fit_options.getModelOptionsPropertyTree().get<bool>(
        "acceptance_correction_active")) {
      matching_acc = lmd_data_facade.getMatchingAcceptances(acceptance_pool,
          lmd_data);

      for (auto const& acc : matching_acc) {
        // set acc in factory
        model_factory.setAcceptance(acc);

        fitElasticPPbar(lmd_data);

        data_bundle.addFittedElasticData(lmd_data);
        data_bundle.attachAcceptanceToCurrentData(
            model_factory.getAcceptance());
        if (fit_options.getModelOptionsPropertyTree().get<bool>(
            "resolution_smearing_active"))
          data_bundle.attachResolutionMapDataToCurrentData(
              *resolution_map_pool.begin());
      }
    } else {
      fitElasticPPbar(lmd_data);

      data_bundle.addFittedElasticData(lmd_data);
    }
    data_bundle.addCurrentDataBundleToList();
    data_bundle.printInfo();
  }

  return data_bundle;
}

void PndLmdFitFacade::fitElasticPPbar(PndLmdAngularData &lmd_data) {
// generate model

  PndLmdFitOptions fit_options(createFitOptions(lmd_data));

  shared_ptr<Model> model = generateModel(lmd_data, fit_options);

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
    integral_data = calcHistIntegral(lmd_data.get2DHistogram(), range);
  else
    integral_data = calcHistIntegral(lmd_data.get1DHistogram(), range);
  std::cout << "calculating model integral..." << std::endl;
  double integral_func = model->Integral(range, 1e-1);
  double binning_factor = lmd_data.getBinningFactor(fit_dimension);
  double lumi_start = integral_data / integral_func / binning_factor;
  cout << "binning factor: " << binning_factor << endl;
  cout << "integral (model): " << integral_func << endl;
  cout << integral_data << " / " << integral_func * binning_factor << endl;
  cout << "Using start luminosity: " << lumi_start << endl;
  model->getModelParameterSet().setModelParameterValue("luminosity",
      lumi_start);

// create minimizer instance with control parameter
  shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());
  //shared_ptr<CeresMinimizer> ceres_minimizer(new CeresMinimizer());

  model_fit_facade.setMinimizer(minuit_minimizer);
  //std::vector<std::string> scan_var_names = {"gauss_sigma_var1", "gauss_sigma_var2"};
  //scanEstimatorSpace(lmd_data, fit_options, scan_var_names);

  doFit(lmd_data, fit_options);
}

shared_ptr<Model> PndLmdFitFacade::generateModel(
    const PndLmdAngularData &lmd_data) {
  PndLmdFitOptions fit_options(createFitOptions(lmd_data));

  return generateModel(lmd_data, fit_options);
}

shared_ptr<Model> PndLmdFitFacade::generateModel(
    const PndLmdAngularData &lmd_data, const PndLmdFitOptions &fit_options) {
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

  return model;
}

void PndLmdFitFacade::scanEstimatorSpace(PndLmdHistogramData &lmd_hist_data,
    const PndLmdFitOptions &fit_options, const std::vector<std::string> &variable_names) {

  cout << "Scanning estimator space with following fit options:" << endl;
  cout << fit_options << endl;

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

  Data scanned_data = model_fit_facade.scanEstimatorSpace(variable_names);

  // lets make a 2d plot here
  TFile f("div_likelihood_scan.root", "RECREATE");
  //TH2D hist2d("name", "div likelihood scan", 60, 0.0000997, 0.000103, 60, 0.000194, 0.000206);
  unsigned int bins(scanned_data.getData().size());
  TVectorD datax(bins);
  TVectorD datay(bins);
  TVectorD dataz(bins);
  auto& datapoints = scanned_data.getData();
  for(unsigned int i = 0; i < scanned_data.getData().size(); ++i) {
    datax[i] = datapoints[i].getBinnedDataPoint()->bin_center_value[0];
    datay[i] = datapoints[i].getBinnedDataPoint()->bin_center_value[1];
    dataz[i] = datapoints[i].getBinnedDataPoint()->z;
  }
  datax.Write("xdata");
  datay.Write("ydata");
  dataz.Write("zdata");
}


void PndLmdFitFacade::doFit(PndLmdHistogramData &lmd_hist_data,
    const PndLmdFitOptions &fit_options) {

  cout << "Attempting to perform fit with following fit options:" << endl;
  cout << fit_options << endl;

  ModelFitResult fit_result;
//first check if this model with the fit options have already been fitted
  /*ModelFitResult fit_result = lmd_hist_data.getFitResult(fit_options);
   if (fit_result.getFitParameters().size() > 0) {
   cout << "Fit was already performed! Skipping..." << endl;
   return;
   }*/

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

  CALLGRIND_START_INSTRUMENTATION;
  fit_result = model_fit_facade.Fit();
  CALLGRIND_STOP_INSTRUMENTATION;
  CALLGRIND_DUMP_STATS;

// store fit results
  cout << "Adding fit result to storage..." << endl;

  lmd_hist_data.addFitResult(fit_options, fit_result);
  cout << "fit storage now contains "
      << lmd_hist_data.getFitResults().at(fit_options).size() << " entries!"
      << endl;
}

void PndLmdFitFacade::fitVertexData(
    std::vector<PndLmdHistogramData> &lmd_data) {
  for (unsigned int i = 0; i < lmd_data.size(); i++) {
    cout << "Fitting vertex distribution " << lmd_data[i].getName() << endl;

    PndLmdFitOptions fit_options(createFitOptions(lmd_data[i]));

    // skip 2d data
    if (lmd_data[i].getSecondaryDimension().is_active) {
      std::cout << "WARNING: 2d vertex fits are not possible\n";
      continue;
    }

    // get histogram
    const TH1D* hist = lmd_data[i].get1DHistogram();
    if (hist->Integral() < 400) {
      std::cout
          << "WARNING: not performing fit due to low statistics of the vertex distribution (below 400 events)\n";
      continue;
    }

    ptree sim_params(lmd_data[i].getSimulationParametersPropertyTree());

    double sigma_range(1.0);

    DataStructs::DimensionRange old_range = fit_options.est_opt.getFitRangeX();
    double ideal_sigma;
    if (lmd_data[i].getPrimaryDimension().dimension_options.track_type
        == LumiFit::MC) {
      DataStructs::DimensionRange fit_range;

      if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
          == LumiFit::X) {
        ideal_sigma = sim_params.get<double>("ip_standard_deviation_x");
        fit_range.range_low = sim_params.get<double>("ip_mean_x")
            - sigma_range * ideal_sigma;
        fit_range.range_high = sim_params.get<double>("ip_mean_x")
            + sigma_range * ideal_sigma;
      } else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
          == LumiFit::Y) {
        ideal_sigma = sim_params.get<double>("ip_standard_deviation_y");
        fit_range.range_low = sim_params.get<double>("ip_mean_y")
            - sigma_range * ideal_sigma;
        fit_range.range_high = sim_params.get<double>("ip_mean_y")
            + sigma_range * ideal_sigma;
      } else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
          == LumiFit::Z) {
        ideal_sigma = sim_params.get<double>("ip_standard_deviation_z");
        fit_range.range_low = sim_params.get<double>("ip_mean_z")
            - sigma_range * ideal_sigma;
        fit_range.range_high = sim_params.get<double>("ip_mean_z")
            + sigma_range * ideal_sigma;
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
            - sigma_range * ideal_sigma;
        fit_range.range_high = sim_params.get<double>("ip_mean_x")
            + sigma_range * ideal_sigma;
      } else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
          == LumiFit::Y) {
        ideal_sigma = hist->GetRMS(1);
        fit_range.range_low = sim_params.get<double>("ip_mean_y")
            - sigma_range * ideal_sigma;
        fit_range.range_high = sim_params.get<double>("ip_mean_y")
            + sigma_range * ideal_sigma;
      } else if (lmd_data[i].getPrimaryDimension().dimension_options.dimension_type
          == LumiFit::Z) {
        ideal_sigma = hist->GetRMS(1);
        fit_range.range_low = sim_params.get<double>("ip_mean_z")
            - sigma_range * ideal_sigma;
        fit_range.range_high = sim_params.get<double>("ip_mean_z")
            + sigma_range * ideal_sigma;
      }

      //fit_range.is_active = true;
      //fit_options.est_opt.setFitRangeX(fit_range);
    }

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
    shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer(1));

    model_fit_facade.setMinimizer(minuit_minimizer);

    doFit(lmd_data[i], fit_options);
  }
}
