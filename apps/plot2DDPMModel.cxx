#include "ui/PndLmdPlotter.h"
#include "ui/PndLmdFitFacade.h"
#include "ui/PndLmdDataFacade.h"

#include "ui/PndLmdRuntimeConfiguration.h"

#include <iostream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void plot2DModel(string input_file_dir, string config_file_url,
    string acceptance_file_dir, unsigned int nthreads) {

  boost::chrono::thread_clock::time_point start =
      boost::chrono::thread_clock::now();

  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();
  lmd_runtime_config.setNumberOfThreads(nthreads);

  lmd_runtime_config.setGeneralConfigDirectory(config_file_url);
  boost::filesystem::path fit_config_path(config_file_url);
  lmd_runtime_config.readFitConfig(fit_config_path.filename().string());

  lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
  lmd_runtime_config.setAcceptanceResolutionInputDirectory(acceptance_file_dir);

  lmd_runtime_config.setElasticDataName("lmd_data_.*of1.root");
  lmd_runtime_config.setAccDataName("lmd_acc_data_.*of1.root");
  lmd_runtime_config.setResDataName("lmd_res_data_.*of1.root");
  lmd_runtime_config.setVertexDataName("lmd_vertex_data_.*of1.root");

  LumiFit::LmdDimensionOptions data_dim_opt;
  data_dim_opt.track_type = LumiFit::MC;
  data_dim_opt.dimension_type = LumiFit::THETA_X;

  // get lmd data and objects from files
  PndLmdDataFacade lmd_data_facade;
  vector<PndLmdAngularData> my_lmd_data_vec = lmd_data_facade.getElasticData();

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(data_dim_opt);
  my_lmd_data_vec = lmd_data_facade.filterData<PndLmdAngularData>(
      my_lmd_data_vec, filter);

  vector<PndLmdAcceptance> my_lmd_acc_vec = lmd_data_facade.getAcceptanceData();
  vector<PndLmdHistogramData> all_lmd_res = lmd_data_facade.getResolutionData();
  vector<PndLmdMapData> all_lmd_res_map = lmd_data_facade.getMapData();
  // ------------------------------------------------------------------------

  // start fitting
  PndLmdFitFacade lmd_fit_facade;
  // add acceptance data to pools
  // the corresponding acceptances to the data will automatically be taken
  // if not found then this fit is skipped
  lmd_fit_facade.addAcceptencesToPool(my_lmd_acc_vec);
  lmd_fit_facade.addResolutionsToPool(all_lmd_res);
  lmd_fit_facade.addResolutionMapsToPool(all_lmd_res_map);

  std::cout << "number of data objects: " << my_lmd_data_vec.size()
      << std::endl;
  auto model = lmd_fit_facade.generateModel(my_lmd_data_vec[0]);

  const double theta_min = -0.013;
  const double theta_max = 0.013;

  DataStructs::DimensionRange plot_range_thx(theta_min, theta_max);
  DataStructs::DimensionRange plot_range_thy(theta_min, theta_max);

  // integral - just for testing purpose
  /*std::vector<DataStructs::DimensionRange> int_range;
   DataStructs::DimensionRange dr_th(0.001, 0.01);
   int_range.push_back(dr_th);
   DataStructs::DimensionRange dr_phi(-TMath::Pi(), TMath::Pi());
   int_range.push_back(dr_phi);
   double integral_value = model->Integral(int_range, 1e-3);
   std::cout << "integral: " << integral_value << std::endl;*/

  //plot result
  std::stringstream filepath;
  filepath << std::getenv("HOME") << "/plots/model_mc_2d.root";
  TFile f(filepath.str().c_str(), "RECREATE");

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

  NeatPlotting::GraphAndHistogramHelper hist_helper;
  model_hist = hist_helper.rescaleAxis(model_hist, 1000.0, 1000.0);
  model_hist->GetXaxis()->SetTitle("\theta_{x} /mrad");
  model_hist->GetYaxis()->SetTitle("\theta_{y} /mrad");


  model_hist->Write("model");

  boost::chrono::thread_clock::time_point stop =
      boost::chrono::thread_clock::now();
  std::cout << "duration: "
      << boost::chrono::duration_cast<boost::chrono::milliseconds>(stop - start).count()
      << " ms\n";
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
    plot2DModel(data_path, config_path, acc_path, nthreads);
  else
    displayInfo();
  return 0;
}

