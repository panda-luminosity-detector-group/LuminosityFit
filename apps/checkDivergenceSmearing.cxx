#include "ui/PndLmdPlotter.h"
#include "models2d/BoxModel2D.h"
#include "models2d/GaussianModel2D.h"

#include "model/PndLmdDivergenceSmearingModel2D.h"
#include "model/PndLmdDifferentialSmearingConvolutionModel2D.h"

#include "ui/PndLmdFitFacade.h"
#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdRuntimeConfiguration.h"

#include <iostream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

#include "callgrind.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void plotSmeared2DModel(string input_file_dir, string config_file_url,
    string acceptance_file_dir, unsigned int nthreads) {
  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();
  lmd_runtime_config.setNumberOfThreads(nthreads);

  std::stringstream model_name;

  //Just take very simple box model
  //model_name << "box_model";
  //shared_ptr<Model2D> signal_model(new BoxModel2D(model_name.str()));
  model_name << "gaus_model";
  shared_ptr<Model2D> signal_model(new GaussianModel2D(model_name.str()));

  // at this point we introduce "non numerical" discretization from the data
  // thats why the data object is required here

  // make the binning twice as fine
  LumiFit::LmdDimension temp_prim_dim;
  temp_prim_dim.bins = 100;
  temp_prim_dim.dimension_range.setRangeLow(-5.0);
  temp_prim_dim.dimension_range.setRangeHigh(5.0);
  temp_prim_dim.calculateBinSize();
  LumiFit::LmdDimension temp_sec_dim;
  temp_sec_dim.bins = 100;
  temp_sec_dim.dimension_range.setRangeLow(-5.0);
  temp_sec_dim.dimension_range.setRangeHigh(5.0);
  temp_sec_dim.calculateBinSize();

  shared_ptr<GaussianModel2D> divergence_model(
      new GaussianModel2D("divergence_model", 4.0));

  shared_ptr<PndLmdDivergenceSmearingModel2D> divergence_smearing_model(
      new PndLmdDivergenceSmearingModel2D(divergence_model, temp_prim_dim,
          temp_sec_dim));

  model_name << "_div_smeared";

  shared_ptr<PndLmdDifferentialSmearingConvolutionModel2D> div_smeared_model(
      new PndLmdDifferentialSmearingConvolutionModel2D(model_name.str(),
          signal_model, divergence_smearing_model, temp_prim_dim, temp_sec_dim,
          PndLmdRuntimeConfiguration::Instance().getNumberOfThreads()));
  div_smeared_model->injectModelParameter(
      divergence_model->getModelParameterSet().getModelParameter(
          "gauss_sigma_var1"));
  div_smeared_model->injectModelParameter(
      divergence_model->getModelParameterSet().getModelParameter(
          "gauss_sigma_var2"));

  // set the parameter values
  //signal_model->getModelParameterSet().freeModelParameter("tilt_x");
  //signal_model->getModelParameterSet().freeModelParameter("tilt_y");
  /* signal_model->getModelParameterSet().setModelParameterValue("lower_edge_var1",
   -2.0);
   signal_model->getModelParameterSet().setModelParameterValue("upper_edge_var1",
   2.0);
   signal_model->getModelParameterSet().setModelParameterValue("lower_edge_var2",
   -2.0);
   signal_model->getModelParameterSet().setModelParameterValue("upper_edge_var2",
   2.0);*/

  signal_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var1", 1.12);
  signal_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var2", 1.12);
  signal_model->getModelParameterSet().setModelParameterValue("gauss_mean_var1",
      0.0);
  signal_model->getModelParameterSet().setModelParameterValue("gauss_mean_var2",
      0.0);

  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var1", 0.15);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_sigma_var2", 0.15);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_mean_var1", 0.0);
  divergence_model->getModelParameterSet().setModelParameterValue(
      "gauss_mean_var2", 0.0);


  double x[2] = { 0.0, 0.0 };
    std::cout << x[0] << "," << x[1] << ": " << signal_model->evaluate(x)
        << std::endl;
    x[0] = 1.12;
    x[1] = 0.0;
    std::cout << x[0] << "," << x[1] << ": " << signal_model->evaluate(x)
        << std::endl;
    x[1] = 1.12;
    x[0] = 0.0;
    std::cout << x[0] << "," << x[1] << ": " << signal_model->evaluate(x)
        << std::endl;


  //CALLGRIND_START_INSTRUMENTATION;
  div_smeared_model->init();
  div_smeared_model->getModelParameterSet().printInfo();
  //CALLGRIND_STOP_INSTRUMENTATION;
  //CALLGRIND_DUMP_STATS;

  x[0] = 0.0;
  x[1] = 0.0;
  std::cout << x[0] << "," << x[1] << ": " << div_smeared_model->evaluate(x)
      << std::endl;
  x[0] = 1.13;
  x[1] = 0.0;
  std::cout << x[0] << "," << x[1] << ": " << div_smeared_model->evaluate(x)
      << std::endl;
  x[0] = 0.0;
  x[1] = 1.13;
  std::cout << x[0] << "," << x[1] << ": " << div_smeared_model->evaluate(x)
      << std::endl;
  x[0] = 1.13;
  x[1] = 1.13;
  std::cout << x[0] << "," << x[1] << ": " << div_smeared_model->evaluate(x)
      << std::endl;

  //plot smeared model
  const double var_min = -10.0;
  const double var_max = 10.0;

  DataStructs::DimensionRange plot_range_thx(var_min, var_max);
  DataStructs::DimensionRange plot_range_thy(var_min, var_max);

  ROOTPlotter root_plotter;

  ModelVisualizationProperties1D vis_prop_th;
  vis_prop_th.setPlotRange(plot_range_thx);
  vis_prop_th.setEvaluations(500);

  ModelVisualizationProperties1D vis_prop_phi;
  vis_prop_phi.setPlotRange(plot_range_thy);
  vis_prop_phi.setEvaluations(500);

  std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D> vis_prop_pair =
      std::make_pair(vis_prop_th, vis_prop_phi);

  TH2D* model_hist = root_plotter.createHistogramFromModel2D(div_smeared_model,
      vis_prop_pair);

  TCanvas c;

  model_hist->Draw("colz");
  model_hist->GetXaxis()->SetTitle("x");
  model_hist->GetYaxis()->SetTitle("y");

  std::stringstream strstream;
  strstream << "div_smeared_test_plot.pdf";
  c.SaveAs(strstream.str().c_str());

  lmd_runtime_config.setNumberOfThreads(nthreads);

  /*lmd_runtime_config.readFitConfigFile(config_file_url);

   lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
   lmd_runtime_config.setAcceptanceResolutionInputDirectory(acceptance_file_dir);

   // ============================== read data ============================== //

   // get lmd data and objects from files
   PndLmdDataFacade lmd_data_facade;

   std::vector<PndLmdAngularData> my_lmd_data_vec =
   lmd_data_facade.getElasticData();

   // filter out specific data
   LumiFit::LmdDimensionOptions lmd_dim_opt;
   lmd_dim_opt.dimension_type = LumiFit::THETA_X;
   lmd_dim_opt.track_type = LumiFit::MC_ACC;
   LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
   my_lmd_data_vec = lmd_data_facade.filterData<PndLmdAngularData>(
   my_lmd_data_vec, filter);

   std::vector<PndLmdAcceptance> my_lmd_acc_vec =
   lmd_data_facade.getAcceptanceData();
   std::vector<PndLmdHistogramData> all_lmd_res =
   lmd_data_facade.getResolutionData();

   // ------------------------------------------------------------------------

   // start fitting
   PndLmdFitFacade lmd_fit_facade;
   // add acceptance data to pools
   // the corresponding acceptances to the data will automatically be taken
   // if not found then this fit is skipped
   lmd_fit_facade.addAcceptencesToPool(my_lmd_acc_vec);
   lmd_fit_facade.addResolutionsToPool(all_lmd_res);

   shared_ptr<Model> model = lmd_fit_facade.createModel2D(my_lmd_data_vec[0]);

   //plot smeared model
   const double var_min = -1e-2;
   const double var_max = 1e-2;

   DataStructs::DimensionRange plot_range_thx(var_min, var_max);
   DataStructs::DimensionRange plot_range_thy(var_min, var_max);

   ROOTPlotter root_plotter;

   ModelVisualizationProperties1D vis_prop_th;
   vis_prop_th.setPlotRange(plot_range_thx);
   vis_prop_th.setEvaluations(200);

   ModelVisualizationProperties1D vis_prop_phi;
   vis_prop_phi.setPlotRange(plot_range_thy);
   vis_prop_phi.setEvaluations(200);

   std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D> vis_prop_pair =
   std::make_pair(vis_prop_th, vis_prop_phi);

   TH2D* model_hist = root_plotter.createHistogramFromModel2D(model,
   vis_prop_pair);

   TCanvas c;

   //c.SetLogz(1);
   model_hist->Draw("colz");
   model_hist->GetXaxis()->SetTitle("x");
   model_hist->GetYaxis()->SetTitle("y");

   std::stringstream strstream;
   strstream << "div_smeared_test_plot.pdf";
   c.SaveAs(strstream.str().c_str());*/
}

void displayInfo() {
  // display info
  cout << "Required arguments are: " << endl;
  cout << "-d [path to data]" << endl;
  cout << "-c [path to config file] " << endl;
  cout << "Optional arguments are: " << endl;
  cout << "-m [number of threads]" << endl;
  cout << "-a [path to box gen data] (acceptance)" << endl;
}

int main(int argc, char* argv[]) {
  string data_path;
  string acc_path("");
  string config_path("");
  unsigned int nthreads(1);
  bool is_data_set(false), is_config_set(false), is_acc_set(false),
      is_nthreads_set(false);

  int c;

  while ((c = getopt(argc, argv, "hc:a:m:d:")) != -1) {
    switch (c) {
    case 'a':
      acc_path = optarg;
      is_acc_set = true;
      break;
    case 'c':
      config_path = optarg;
      is_config_set = true;
      break;
    case 'd':
      data_path = optarg;
      is_data_set = true;
      break;
    case 'm':
      nthreads = atoi(optarg);
      is_nthreads_set = true;
      break;
    case '?':
      if (optopt == 'm' || optopt == 'd' || optopt == 'a' || optopt == 'c')
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

  if (is_data_set && is_config_set)
    plotSmeared2DModel(data_path, config_path, acc_path, nthreads);
  else
    displayInfo();
  return 0;
}

