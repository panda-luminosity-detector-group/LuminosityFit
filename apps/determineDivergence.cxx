//J.Li, 14.07.2023
//Script for external divergence determination, derived from "checkDivergenceSmearing.cxx"

#include "models2d/BoxModel2D.h"
#include "models2d/GaussianModel2D.h"
#include "ui/PndLmdPlotter.h"

#include "model/CachedModel2D.h"
#include "model/PndLmdDifferentialSmearingConvolutionModel2D.h"
#include "model/PndLmdDivergenceSmearingModel2D.h"

#include "fit/estimatorImpl/LogLikelihoodEstimator.h"
#include "fit/minimizerImpl/ROOT/ROOTMinimizer.h"

#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdFitFacade.h"
#include "ui/PndLmdRuntimeConfiguration.h"

#include <iostream>
#include <string>

#include "TCanvas.h"

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

void makeSinglePlot(std::shared_ptr<Model2D> model, string name,
                    bool symmetrize = false) {
  // plot smeared model
  double var_min = -4.0;
  double var_max = 4.0;
  if (symmetrize) {
    var_min = -1.96;
    var_max = 1.96;
  }
  DataStructs::DimensionRange plot_range_thx(var_min, var_max);
  DataStructs::DimensionRange plot_range_thy(var_min, var_max);
  ROOTPlotter root_plotter;

  ModelVisualizationProperties1D vis_prop_th;
  vis_prop_th.setPlotRange(plot_range_thx);
  vis_prop_th.setEvaluations(200);

  ModelVisualizationProperties1D vis_prop_phi;
  vis_prop_phi.setPlotRange(plot_range_thy);
  vis_prop_phi.setEvaluations(200);

  std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D>
      vis_prop_pair = std::make_pair(vis_prop_th, vis_prop_phi);

  TH2D *model_hist =
      root_plotter.createHistogramFromModel2D(model, vis_prop_pair);

  model_hist->Write(name.c_str());
}

void plotSmeared2DModel(unsigned int nthreads, unsigned int scale_,
                        unsigned int binning_) {
  PndLmdRuntimeConfiguration &lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();
  lmd_runtime_config.setNumberOfThreads(nthreads);

  unsigned int scale(scale_);
  const double var_max = 10.0 / scale;
  unsigned int binning = binning_ / scale;

  double data_sigma_x = 3.0 / scale;
  double data_sigma_y = 2.5 / scale;
  double smear_sigma_x = 0.5 / scale;
  double smear_sigma_y = 0.1 / scale;
  double model_sigma_x =
      sqrt(data_sigma_x * data_sigma_x - smear_sigma_x * smear_sigma_x);
  double model_sigma_y =
      sqrt(data_sigma_y * data_sigma_y - smear_sigma_y * smear_sigma_y);

  // at this point we introduce "non numerical" discretization from the data
 // thats why the data object is required here

  // make the binning twice as fine
  LumiFit::LmdDimension temp_prim_dim;
  temp_prim_dim.bins = binning;
  temp_prim_dim.dimension_range.setRangeLow(-var_max);
  temp_prim_dim.dimension_range.setRangeHigh(var_max);
  temp_prim_dim.calculateBinSize();
  LumiFit::LmdDimension temp_sec_dim;
  temp_sec_dim.bins = binning;
  temp_sec_dim.dimension_range.setRangeLow(-var_max);
  temp_sec_dim.dimension_range.setRangeHigh(var_max);
  temp_sec_dim.calculateBinSize();

  std::stringstream model_name;

  // Just take very simple box model
  // model_name << "box_model";
  // std::shared_ptr<Model2D> signal_model(new BoxModel2D(model_name.str()));
  model_name << "gaus_model";
  std::shared_ptr<Model2D> signal_model(
      new GaussianModel2D(model_name.str(), 3.0));

  signal_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var1", data_sigma_x);
  signal_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var2", data_sigma_y);
  signal_model->getModelParameterSet().setModelParameterValue("gauss_mean_var1",
                                                              0.0);
  signal_model->getModelParameterSet().setModelParameterValue("gauss_mean_var2",
                                                              0.0);
  signal_model->getModelParameterSet().setModelParameterValue("gauss_rho", 0.0);

  std::shared_ptr<Model2D> cached_signal_model(new CachedModel2D(
      model_name.str(), signal_model, temp_prim_dim, temp_sec_dim));
  cached_signal_model->updateModel();

  DataStructs::DimensionRange plot_range_thx(-var_max, var_max);
  DataStructs::DimensionRange plot_range_thy(-var_max, var_max);

  ROOTPlotter root_plotter;

  ModelVisualizationProperties1D vis_prop_th;
  vis_prop_th.setPlotRange(plot_range_thx);
  vis_prop_th.setEvaluations(binning);

  ModelVisualizationProperties1D vis_prop_phi;
  vis_prop_phi.setPlotRange(plot_range_thy);
  vis_prop_phi.setEvaluations(binning);

  std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D>
      vis_prop_pair = std::make_pair(vis_prop_th, vis_prop_phi);

  TH2D *signal_hist = root_plotter.createHistogramFromModel2D(
      cached_signal_model, vis_prop_pair);

  model_name << "_cached";

  std::shared_ptr<GaussianModel2D> divergence_model(
      new GaussianModel2D("divergence_model", 5.0));

  std::shared_ptr<PndLmdDivergenceSmearingModel2D> divergence_smearing_model(
      new PndLmdDivergenceSmearingModel2D(divergence_model, temp_prim_dim,
                                          temp_sec_dim));

  model_name << "_div_smeared";

  std::shared_ptr<PndLmdDifferentialSmearingConvolutionModel2D>
      div_smeared_model(new PndLmdDifferentialSmearingConvolutionModel2D(
          model_name.str(), cached_signal_model, divergence_smearing_model,
          temp_prim_dim, temp_sec_dim,
          PndLmdRuntimeConfiguration::Instance().getNumberOfThreads()));
  div_smeared_model->injectModelParameter(
      divergence_model->getModelParameterSet().getModelParameter(
          "gauss_sigma_var1"));
  div_smeared_model->injectModelParameter(
      divergence_model->getModelParameterSet().getModelParameter(
          "gauss_sigma_var2"));

  // set the parameter values
  signal_model->getModelParameterSet().freeModelParameter("gauss_sigma_var1");
  signal_model->getModelParameterSet().freeModelParameter("gauss_sigma_var2");
  signal_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var1", model_sigma_x);
  signal_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var2", model_sigma_y);
  signal_model->getModelParameterSet()
      .getModelParameter("gauss_sigma_var1")
      ->setParameterFixed(true);
  signal_model->getModelParameterSet()
      .getModelParameter("gauss_sigma_var2")
      ->setParameterFixed(true);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var1", smear_sigma_x);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var2", smear_sigma_y);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_mean_var1", 0.0);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_mean_var2", 0.0);
  divergence_model->getModelParameterSet().setModelParameterValue("gauss_rho",
                                                                  0.0);

  div_smeared_model->init();

  ModelFitFacade model_fit_facade;

  std::shared_ptr<ModelEstimator> estimator(new LogLikelihoodEstimator());

  estimator->setNumberOfThreads(lmd_runtime_config.getNumberOfThreads());

  model_fit_facade.setEstimator(estimator);
  EstimatorOptions est_opt;
  model_fit_facade.setEstimatorOptions(est_opt);

  lmd_runtime_config.setElasticDataInputDirectory("/lustre/miifs05/scratch/him-specf/paluma/jinxin/Lmddata/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_-0.13_0.03_0.13_0.0    6_0.05_0.38/beam_grad_XYDXDY_0.00017_8.999999999999999e-05_0.00015_0.00013000000000000002/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/merge_data/");
  //lmd_runtime_config.setAcceptanceResolutionInputDirectory("/lustre/miifs05/scratch/him-specf/paluma/jinxin/Lmddata/plab_4.06GeV/resAcc-box_theta_2.56-13.14mrad_recoil_corrected/ip_offset_XYZDXDYDZ_-0.136_    0.031_0.13_0.06_0.05_0.38/beam_grad_XYDXDY_0.00017_8.999999999999999e-05_0.00015_0.00013000000000000002/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_50/binning_300/merge_dat    a/");
  lmd_runtime_config.setElasticDataName("lmd_data_.*of1.root");
  PndLmdDataFacade lmd_data_facade;
  //std::vector<PndLmdHistogramData> lmd_data_vec = lmd_data_facade.getVertexData();
  std::vector<PndLmdAngularData> lmd_data_vec = lmd_data_facade.getElasticData();
  //PndLmdHistogramData lmd_data = lmd_data_vec[0];
  PndLmdAngularData lmd_data = lmd_data_vec[0];
  PndLmdFitFacade lmd_fit_facade;
  std::shared_ptr<Data> data = lmd_fit_facade.createData2D(lmd_data);

  model_fit_facade.setData(data);

  model_fit_facade.setModel(div_smeared_model);
  signal_model->getModelParameterSet().freeModelParameter("gauss_amplitude");
  divergence_model->getModelParameterSet().freeModelParameter(
      "gauss_sigma_var1");
  divergence_model->getModelParameterSet().freeModelParameter(
      "gauss_sigma_var2");
  div_smeared_model->getModelParameterSet().printInfo();

  std::shared_ptr<ROOTMinimizer> root_minimizer(new ROOTMinimizer);
  model_fit_facade.setMinimizer(root_minimizer);

  ModelFitResult fit_result = model_fit_facade.Fit();

  // store fit results
  cout << "Adding fit result to storage..." << endl;
  cout << "div_x = " << smear_sigma_x << " " << model_sigma_x << " " << data_sigma_x <<endl;
  // fit_result

  TH2D *result_hist =
     root_plotter.createHistogramFromModel2D(div_smeared_model, vis_prop_pair);

  TFile f("results.root", "RECREATE");
  result_hist->Write("model");
  signal_hist->Write("data");
}

int main(int argc, char *argv[]) {
  unsigned int nthreads(1);
  bool is_nthreads_set(false);
  unsigned int scale(10);
  unsigned int binning(240);
  int c;

  while ((c = getopt(argc, argv, "hm:s:b:")) != -1) {
    switch (c) {
    case 'm':
      nthreads = atoi(optarg);
      is_nthreads_set = true;
      break;
    case 's':
      scale = atoi(optarg);
      break;
    case 'b':
      binning = atoi(optarg);
      break;
    case '?':
      if (optopt == 'm' || optopt == 's' || optopt == 'b')
        cerr << "Option -" << optopt << " requires an argument." << endl;
      else if (isprint(optopt))
        cerr << "Unknown option -" << optopt << "." << endl;
      else
        cerr << "Unknown option character" << optopt << "." << endl;
      return 1;
    case 'h':
      displayInfo();
      return 1;
    default:
      return 1;
    }
  }

  if (is_nthreads_set)
    plotSmeared2DModel(nthreads, scale, binning);
  else
   displayInfo();
  return 0;
}
