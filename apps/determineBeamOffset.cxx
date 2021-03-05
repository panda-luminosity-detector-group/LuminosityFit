#include "LumiFitStructs.h"
#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdFitFacade.h"

#include <iostream>
#include <sstream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include "boost/property_tree/json_parser.hpp"
#include <boost/chrono/thread_clock.hpp>

#include "TFile.h"

using boost::property_tree::ptree;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

void determineBeamOffset(string input_file_dir, string config_file_url,
                         unsigned int nthreads) {

  boost::chrono::thread_clock::time_point start =
      boost::chrono::thread_clock::now();

  PndLmdRuntimeConfiguration &lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();
  lmd_runtime_config.setNumberOfThreads(nthreads);

  // set general config path
  lmd_runtime_config.setGeneralConfigDirectory(config_file_url);
  boost::filesystem::path fit_config_path(config_file_url);
  lmd_runtime_config.readFitConfig(fit_config_path.filename().string());

  lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
  lmd_runtime_config.setVertexDataName("lmd_vertex_data_.*of1.root");

  // ============================== read data ============================== //

  // get lmd data and objects from files
  PndLmdDataFacade lmd_data_facade;

  vector<PndLmdHistogramData> my_vertex_vec = lmd_data_facade.getVertexData();

  /*// filter out specific data
   LumiFit::LmdDimensionOptions lmd_dim_opt;
   lmd_dim_opt.dimension_type = LumiFit::THETA_X;
   lmd_dim_opt.track_type = LumiFit::RECO;

   const boost::property_tree::ptree& fit_config_ptree =
   lmd_runtime_config.getFitConfigTree();
   if
   (fit_config_ptree.get<bool>("fit.fit_model_options.acceptance_correction_active")
   == true) { lmd_dim_opt.track_type = LumiFit::MC_ACC; if
   (fit_config_ptree.get<bool>("fit.fit_model_options.resolution_smearing_active")
   == true) lmd_dim_opt.track_type = LumiFit::RECO;
   }

   LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
   my_lmd_data_vec = lmd_data_facade.filterData<PndLmdAngularData>(
   my_lmd_data_vec, filter);*/

  // ----------------------------------------------------------------------
  PndLmdFitFacade lmd_fit_facade;

  // do fits
  lmd_fit_facade.fitVertexData(my_vertex_vec);

  // save data
  std::cout << "Saving data...." << std::endl;
  // output file
  std::stringstream hs;
  hs << input_file_dir << "/reco_ip.json";

  ptree fit_result_ptree;
  for (auto const &vertex_data : my_vertex_vec) {
    if (vertex_data.getPrimaryDimension().dimension_options.track_type ==
        LumiFit::RECO) {
      string label("");
      if (vertex_data.getPrimaryDimension().dimension_options.dimension_type ==
          LumiFit::X)
        label = "ip_x";
      else if (vertex_data.getPrimaryDimension()
                   .dimension_options.dimension_type == LumiFit::Y)
        label = "ip_y";

      if (label != "") {
        if (vertex_data.getFitResults().size() > 0) {
          std::cout << label << std::endl;
          auto const &fit_res = vertex_data.getFitResults().begin()->second[0];
          double value = fit_res.getFitParameter("gauss_mean").value;
          fit_result_ptree.add(label, value);
        }
      }
    }
  }
  // add rec ip_z as 0.0
  fit_result_ptree.add("ip_z", 0.0);
  std::cout << "writing result to " << hs.str() << std::endl;
  write_json(hs.str(), fit_result_ptree);

  hs.str("");
  hs << input_file_dir << "/lmd_fitted_vertex_data.root";
  TFile *ffitteddata = new TFile(hs.str().c_str(), "RECREATE");

  for (std::vector<PndLmdHistogramData>::iterator lmd_data_iter =
           my_vertex_vec.begin();
       lmd_data_iter != my_vertex_vec.end(); lmd_data_iter++) {
    std::cout << lmd_data_iter->getName() << ": "
              << lmd_data_iter->getFitResults().size() << std::endl;
    if (lmd_data_iter->getFitResults().size() > 0)
      lmd_data_iter->saveToRootFile();
  }

  ffitteddata->Close();
}

void displayInfo() {
  // display info
  cout << "Required arguments are: " << endl;
  cout << "-p [path to data]" << endl;
  cout << "-c [path to config file] " << endl;
  cout << "Optional arguments are: " << endl;
  cout << "-m [number of threads]" << endl;
}

int main(int argc, char *argv[]) {
  string data_path;
  string config_path("");
  unsigned int nthreads(1);
  bool is_data_set(false), is_config_set(false), is_nthreads_set(false);

  int c;

  while ((c = getopt(argc, argv, "hc:m:p:")) != -1) {
    switch (c) {
    case 'c':
      config_path = optarg;
      is_config_set = true;
      break;
    case 'p':
      data_path = optarg;
      is_data_set = true;
      break;
    case 'm':
      nthreads = atoi(optarg);
      is_nthreads_set = true;
      break;
    case '?':
      if (optopt == 'm' || optopt == 'p' || optopt == 'c')
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
    determineBeamOffset(data_path, config_path, nthreads);
  else
    displayInfo();
  return 0;
}
