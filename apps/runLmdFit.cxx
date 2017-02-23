#include "ui/PndLmdRuntimeConfiguration.h"
#include "ui/PndLmdFitFacade.h"
#include "ui/PndLmdDataFacade.h"
#include "data/PndLmdAngularData.h"

#include <iostream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

#include "TFile.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void runLmdFit(string input_file_dir, string config_directory,
    string acceptance_file_dir, string reference_acceptance_file_dir,
    unsigned int nthreads) {

  boost::chrono::thread_clock::time_point start =
      boost::chrono::thread_clock::now();

  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();
  lmd_runtime_config.setNumberOfThreads(nthreads);
  lmd_runtime_config.setGeneralConfigDirectory(config_directory);

  lmd_runtime_config.readAcceptanceOffsetTransformationParameters(
      "offset_trafo_matrix.json");

  // the next line is not really needed later but we have it in there for debuging reasons right now
  // the fit facade uses the divergence values from the sim params file in for start values in case they
  // are not set
  lmd_runtime_config.readSimulationParameters(
      input_file_dir + "/../../../../sim_params.config");

  lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
  lmd_runtime_config.setAcceptanceResolutionInputDirectory(acceptance_file_dir);
  lmd_runtime_config.setReferenceAcceptanceResolutionInputDirectory(
      reference_acceptance_file_dir);

  lmd_runtime_config.setElasticDataName("lmd_data_.*of1.root");
  lmd_runtime_config.setAccDataName("lmd_acc_data_.*of1.root");
  lmd_runtime_config.setResDataName("lmd_res_data_.*of1.root");
  lmd_runtime_config.setVertexDataName("lmd_vertex_data_.*of1.root");

  // ============================== read data ============================== //

  // get lmd data and objects from files
  PndLmdDataFacade lmd_data_facade;

  PndLmdFitFacade lmd_fit_facade;

  // get vertex data and determine and run ip determination for these
  vector<PndLmdHistogramData> my_vertex_vec = lmd_data_facade.getVertexData();
  // do fits
  lmd_runtime_config.readFitConfig("vertex_fitconfig.json");
  lmd_fit_facade.fitVertexData(my_vertex_vec);
  std::pair<double, double> ip_offsets(0.0, 0.0);
  for (auto const &vertex_data : my_vertex_vec) {
    if (vertex_data.getPrimaryDimension().dimension_options.track_type
        == LumiFit::RECO) {
      if (!vertex_data.getSecondaryDimension().is_active) {
        auto fit_results = vertex_data.getFitResults();
        if (fit_results.size() > 0) {
          if (vertex_data.getPrimaryDimension().dimension_options.dimension_type
              == LumiFit::X) {
            ip_offsets.first = fit_results.begin()->second[0].getFitParameter(
                "gauss_mean").value;
          }
          if (vertex_data.getPrimaryDimension().dimension_options.dimension_type
              == LumiFit::Y) {
            ip_offsets.second = fit_results.begin()->second[0].getFitParameter(
                "gauss_mean").value;
          }
        }
      }
    }
  }
  // ok now we have to set the correct ip offsets to the elastic data sets
  // so compare the data on an abstract level
  vector<PndLmdAngularData> my_lmd_data_vec = lmd_data_facade.getElasticData();
  for (auto &data : my_lmd_data_vec) {
    data.setIPOffsets(ip_offsets);
  }

  lmd_runtime_config.readFitConfig("fitconfig.json");

  // filter out specific data
  LumiFit::LmdDimensionOptions lmd_dim_opt;
  lmd_dim_opt.dimension_type = LumiFit::THETA_X;
  lmd_dim_opt.track_type = LumiFit::RECO;

  const boost::property_tree::ptree& fit_config_ptree =
      lmd_runtime_config.getFitConfigTree();
  if (fit_config_ptree.get<bool>(
      "fit.fit_model_options.acceptance_correction_active") == true) {
    lmd_dim_opt.track_type = LumiFit::MC_ACC;
    if (fit_config_ptree.get<bool>(
        "fit.fit_model_options.resolution_smearing_active") == true)
      lmd_dim_opt.track_type = LumiFit::RECO;
  }

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
  my_lmd_data_vec = lmd_data_facade.filterData<PndLmdAngularData>(
      my_lmd_data_vec, filter);

  LumiFit::Comparisons::NoSecondaryTrackFilter no_cut_on_secondary_filter;
  my_lmd_data_vec = lmd_data_facade.filterData<PndLmdAngularData>(
      my_lmd_data_vec, no_cut_on_secondary_filter);

  vector<PndLmdAcceptance> my_lmd_acc_vec = lmd_data_facade.getAcceptanceData();
  vector<PndLmdHistogramData> all_lmd_res = lmd_data_facade.getResolutionData();
  vector<PndLmdMapData> all_lmd_res_map = lmd_data_facade.getMapData();
  // ------------------------------------------------------------------------

  // start fitting
  // add acceptance data to pools
  // the corresponding acceptances to the data will automatically be taken
  // if not found then this fit is skipped
  lmd_fit_facade.addAcceptencesToPool(my_lmd_acc_vec);
  lmd_fit_facade.addResolutionsToPool(all_lmd_res);
  lmd_fit_facade.addResolutionMapsToPool(all_lmd_res_map);

  // we dont need these data objects anymore
  all_lmd_res_map.clear();
  my_lmd_acc_vec.clear();

  // do actual fits
  PndLmdFitDataBundle fit_result(
      lmd_fit_facade.doLuminosityFits(my_lmd_data_vec));

  // open file via runtime config and save results to file
  // create output file
  std::stringstream hs;
  hs << lmd_runtime_config.getElasticDataInputDirectory().string() << "/"
      << lmd_runtime_config.getFittedElasticDataName();

  fit_result.saveDataBundleToRootFile(hs.str());

  boost::chrono::thread_clock::time_point stop =
      boost::chrono::thread_clock::now();
  std::cout << "duration: "
      << boost::chrono::duration_cast<boost::chrono::milliseconds>(stop - start).count()
          / 60000.0 << " min\n";
}

void displayInfo() {
  // display info
  cout << "Required arguments are: " << endl;
  cout << "-d [path to data]" << endl;
  cout << "-c [path to config file]" << endl;
  cout << "Optional arguments are: " << endl;
  cout << "-m [number of threads]" << endl;
  cout << "-a [path to box gen data] (acceptance)" << endl;
  cout << "-r [path to reference box gen data] (acceptance)" << endl;
}

int main(int argc, char* argv[]) {
  string data_path;
  string acc_path("");
  string config_path("");
  string ref_acc_path("");
  unsigned int nthreads(1);
  bool is_data_set(false), is_config_set(false), is_acc_set(false),
      is_nthreads_set(false);

  int c;

  while ((c = getopt(argc, argv, "hc:a:m:r:d:X:Y:")) != -1) {
    switch (c) {
    case 'a':
      acc_path = optarg;
      is_acc_set = true;
      break;
    case 'c':
      config_path = optarg;
      is_config_set = true;
      break;
    case 'r':
      ref_acc_path = optarg;
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
      if (optopt == 'm' || optopt == 'd' || optopt == 'a' || optopt == 'c'
          || optopt == 'r')
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
    runLmdFit(data_path, config_path, acc_path, ref_acc_path, nthreads);
  else
    displayInfo();
  return 0;
}

