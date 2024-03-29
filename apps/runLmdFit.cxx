#include "data/PndLmdAngularData.h"
#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdFitFacade.h"
#include "ui/PndLmdPlotter.h"
#include "ui/PndLmdRuntimeConfiguration.h"

#include <iostream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "TFile.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

void runLmdFit(string input_file_dir, string fit_config_path,
               string acceptance_file_dir, string reference_acceptance_file_dir,
               unsigned int nthreads, string lumiFitResultPath,
               string recoIPfile) {

  boost::chrono::thread_clock::time_point start =
      boost::chrono::thread_clock::now();

  PndLmdRuntimeConfiguration &lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();
  lmd_runtime_config.setNumberOfThreads(nthreads);
  lmd_runtime_config.setGeneralConfigDirectory(fit_config_path);

  // TODO: why is the file name hard coded?
  lmd_runtime_config.readAcceptanceOffsetTransformationParameters(
      "offset_trafo_matrix.json");

  lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
  lmd_runtime_config.setAcceptanceResolutionInputDirectory(acceptance_file_dir);
  lmd_runtime_config.setReferenceAcceptanceResolutionInputDirectory(
      reference_acceptance_file_dir);

  lmd_runtime_config.setElasticDataName("lmd_data_.*of.*.root");
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

  // TODO: why is the file name hard coded?
  lmd_runtime_config.readFitConfig("vertex_fitconfig.json");
  lmd_fit_facade.fitVertexData(my_vertex_vec);

  // read reco IP from file
  std::pair<double, double> ip_offsets(0.0, 0.0);
  if (recoIPfile == "") {
    std::cout << "WARNING: no reco IP file given, using 0,0 as reco IP\n";
  } else {
    std::cout << "reading reco IP from file: " << recoIPfile << "\n";
    boost::property_tree::ptree reco_ip_ptree;
    read_json(recoIPfile, reco_ip_ptree);
    double reco_ip_x = reco_ip_ptree.get<double>("ip_x");
    double reco_ip_y = reco_ip_ptree.get<double>("ip_y");
    ip_offsets.first = reco_ip_x;
    ip_offsets.second = reco_ip_y;
  }
  // print them just to be sure
  std::cout << "Read reco IP values " << ip_offsets.first << " and "
            << ip_offsets.second << "\n";

  // ok now we have to set the correct ip offsets to the elastic data sets
  // so compare the data on an abstract level
  vector<PndLmdAngularData> all_lmd_data_vec = lmd_data_facade.getElasticData();
  for (auto &data : all_lmd_data_vec) {
    data.setIPOffsets(ip_offsets);
  }

  lmd_runtime_config.readFitConfig(fit_config_path);

  // filter out specific data
  LumiFit::LmdDimensionOptions lmd_dim_opt;
  lmd_dim_opt.dimension_type = LumiFit::THETA_X;
  lmd_dim_opt.track_type = LumiFit::MC;

  const boost::property_tree::ptree &fit_config_ptree =
      lmd_runtime_config.getFitConfigTree();
  if (fit_config_ptree.get<bool>(
          "fit.fit_model_options.acceptance_correction_active") == true) {
    lmd_dim_opt.track_type = LumiFit::MC_ACC;
    if (fit_config_ptree.get<bool>(
            "fit.fit_model_options.resolution_smearing_active") == true)
      lmd_dim_opt.track_type = LumiFit::RECO;
  }

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
  vector<PndLmdAngularData> my_lmd_data_vec =
      lmd_data_facade.filterData<PndLmdAngularData>(all_lmd_data_vec, filter);

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
            << boost::chrono::duration_cast<boost::chrono::milliseconds>(stop -
                                                                         start)
                       .count() /
                   60000.0
            << " min\n";

  // write data to json here

  //  This is surprisingly convoluted.

  // we want the fit Result
  // the stuff we're looking for is in a PndLmdElasticDataBundle
  // we have a PndLmdFitDataBundle, but that contains multiple
  // PndLmdElasticDataBundle
  // and these contain the actual fit_result s again

  auto elasticDataBundles = fit_result.getElasticDataBundles();

  // this code is copied from extractLuminosityvalues.cxx

  LumiFit::PndLmdPlotter lmd_plotter;

  // TODO: wait if there are multiple bundles, don't we also have multiple lumi
  // fits? what about the others?
  for (auto const &data_sample : elasticDataBundles) {
    if (data_sample.getFitResults().size() > 0 &&
        data_sample.getSelectorSet().size() == 0) {
      PndLmdLumiFitResult final_fit_result;
      for (auto const &fit_res_pair : data_sample.getFitResults()) {
        bool div_smeared =
            fit_res_pair.first.getModelOptionsPropertyTree().get<bool>(
                "divergence_smearing_active");
        for (auto const &fit_res : fit_res_pair.second) {
          // usually there is just a single fit result
          if (div_smeared) {
            if (fit_res.getFitStatus() == 0) {
              final_fit_result.setModelFitResult(fit_res);
              // break;
            }
          } else {
            final_fit_result.setModelFitResult(fit_res);
          }

          std::pair<double, double> lumi =
              lmd_plotter.calulateRelDiff(final_fit_result.getLuminosity(),
                                          final_fit_result.getLuminosityError(),
                                          data_sample.getReferenceLuminosity());

          std::cout << "Luminosity Fit Result:\n";
          std::cout << "measured luminosity:"
                    << final_fit_result.getLuminosity() << "\n";
          std::cout << "measured luminosity error:"
                    << final_fit_result.getLuminosityError() << "\n";
          std::cout << "generated luminosity:"
                    << data_sample.getReferenceLuminosity() << "\n";
          std::cout << "relative deviation (%):" << lumi.first << "\n";
          std::cout << "relative deviation error (%):" << lumi.second << "\n";

          // add this to json
          boost::property_tree::ptree lumiJson;
          lumiJson.put("measured_lumi", final_fit_result.getLuminosity());
          lumiJson.put("measured_lumi_error",
                       final_fit_result.getLuminosityError());
          lumiJson.put("generated_lumi", data_sample.getReferenceLuminosity());
          lumiJson.put("relative_deviation_in_percent", lumi.first);
          lumiJson.put("relative_deviation_error_in_percent", lumi.second);
          lumiJson.put("fit_converged",
                       final_fit_result.getModelFitResult().getFitStatus() ==
                           0);

          std::stringstream filename;
          if (lumiFitResultPath == "") {
            filename << input_file_dir;
            filename << "/lumi-values.json";
          } else {
            filename << lumiFitResultPath;
          }
          boost::property_tree::write_json(filename.str(), lumiJson);
        }
      }
    }
  }
}

void displayInfo() {
  // display info
  cout << "Required arguments are: " << endl;
  cout << "-d [abs path to data]" << endl;
  cout << "-c [abs path to fit config file]" << endl;
  cout << "Optional arguments are: " << endl;
  cout << "-m [number of threads]" << endl;
  cout << "-a [abs path to box gen data] (acceptance)" << endl;
  cout << "-r [abs path to reference box gen data] (acceptance)" << endl;
  cout << "-i [abs path to reco IP file]" << endl;
  cout << "-o [abs path to lumi fit result file]" << endl;
}

int main(int argc, char *argv[]) {
  string data_path;
  string acc_path("");
  string fit_config_path("");
  string ref_acc_path("");
  string lumiFitResultPath("");
  string recoIPfile("");

  unsigned int nthreads(1);
  bool is_data_set(false), is_config_set(false), is_acc_set(false),
      is_nthreads_set(false);

  int c;

  while ((c = getopt(argc, argv, "hc:a:m:r:d:X:Y:o:i:")) != -1) {
    switch (c) {
    case 'a':
      acc_path = optarg;
      is_acc_set = true;
      break;
    case 'c':
      fit_config_path = optarg;
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
    case 'o':
      lumiFitResultPath = optarg;
      break;
    case 'i':
      recoIPfile = optarg;
      break;
    case '?':
      if (optopt == 'm' || optopt == 'd' || optopt == 'a' || optopt == 'c' ||
          optopt == 'r')
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
    runLmdFit(data_path, fit_config_path, acc_path, ref_acc_path, nthreads,
              lumiFitResultPath, recoIPfile);
  else
    displayInfo();
  return 0;
}
