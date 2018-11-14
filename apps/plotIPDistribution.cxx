#include "ui/PndLmdPlotter.h"
#include "ui/PndLmdDataFacade.h"

#include <vector>
#include <map>
#include <iostream>

#include "TFile.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TVectorD.h"

#include <boost/filesystem.hpp>

using std::cout;
using std::cerr;
using std::endl;

void plotIPDistribution(const std::vector<std::string>& paths,
    const std::string& output_directory_path,
    const std::string& filter_string) {
  std::cout << "Generating lumi plots for fit results....\n";

// ================================= END CONFIG ================================= //

// A small helper class that helps to construct lmd data objects
  PndLmdDataFacade lmd_data_facade;

  LumiFit::PndLmdPlotter lmd_plotter;

// get all data first
  std::vector<PndLmdHistogramData> all_data;

  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();

  for (unsigned int i = 0; i < paths.size(); i++) {
    // ------ get files -------------------------------------------------------
    std::vector<std::string> file_paths = lmd_data_facade.findFilesByName(
        paths[i], filter_string, "lmd_fitted_vertex_data.root");

    for (unsigned int j = 0; j < file_paths.size(); j++) {
      std::string fullpath = file_paths[j];
      TFile fdata(fullpath.c_str(), "READ");

      // read in data from a root file which will return a map of PndLmdAngularData objects
      std::vector<PndLmdHistogramData> data_vec =
          lmd_data_facade.getDataFromFile<PndLmdHistogramData>(fdata);

      // append all data objects to the end of the corresponding data map vectors
      all_data.insert(all_data.end(), data_vec.begin(), data_vec.end());
    }
  }

// =============================== BEGIN PLOTTING =============================== //

  std::stringstream basepath;
  basepath << output_directory_path;

  if (!boost::filesystem::exists(output_directory_path)) {
    std::cout
        << "The output directory path you specified does not exist! Please make sure you are pointing to an existing directory."
        << std::endl;
    return;
  }

  // first filter data vector for reco type objects only
  LumiFit::Comparisons::DataPrimaryDimensionTrackTypeFilter filter(
      LumiFit::RECO);
  std::vector<PndLmdHistogramData> reco_filtered_vertex_data_vec =
      lmd_data_facade.filterData(all_data, filter);

  // ----------- make overview plot ---------------
  if (true) {
    std::cout << "making overview plot...\n";
    std::stringstream filename;
    filename << basepath.str() << "/";
    filename << "plab_"
        << reco_filtered_vertex_data_vec.begin()->getLabMomentum()
        << "/lumifit_result_ip_xy_overview.root";

    std::pair<TGraphAsymmErrors*, TGraphAsymmErrors*> graphs =
        lmd_plotter.makeIPXYOverviewGraphs(reco_filtered_vertex_data_vec);

    TFile newfile(filename.str().c_str(), "RECREATE");
    graphs.first->Write("ip_xy_reco_fit");
    graphs.second->Write("ip_xy_truth");
  }

  if (false) {
    if (reco_filtered_vertex_data_vec.size() > 0) {
      std::stringstream filepath;
      filepath << basepath.str() << "/";
      filepath << "plab_" << reco_filtered_vertex_data_vec[0].getLabMomentum();

      filepath << "/"
          << lmd_plotter.makeDirName(reco_filtered_vertex_data_vec[0]);
      boost::filesystem::create_directories(filepath.str());
      filepath << "/" << "ip_xy_fit_results_1d.root";

      TFile newfile(filepath.str().c_str(), "RECREATE");

      for (auto const& vertex_data : reco_filtered_vertex_data_vec) {
        if (vertex_data.getPrimaryDimension().dimension_options.dimension_type
            == LumiFit::X) {
          lmd_plotter.makeVertexFitBundle(vertex_data, "x");
        } else if (vertex_data.getPrimaryDimension().dimension_options.dimension_type
            == LumiFit::Y) {
          lmd_plotter.makeVertexFitBundle(vertex_data, "y");
        } else if (vertex_data.getPrimaryDimension().dimension_options.dimension_type
            == LumiFit::Z) {
          lmd_plotter.makeVertexFitBundle(vertex_data, "z");
        }
      }

      /*TODO: boost::property_tree::ptree sim_params(
          reco_filtered_vertex_data_vec[0].getSimulationParametersPropertyTree());

      TVectorD v(2);
      v[0] = sim_params.get<double>("ip_mean_x");
      v[1] = sim_params.get<double>("ip_mean_y");

      v.Write("true_values");*/
    }
  }

  // ================================ END PLOTTING ================================ //
}

void displayInfo() {
  // display info
  cout << "Required arguments are: " << endl;
  cout << "list of directories to be scanned for vertex data" << endl;
}

int main(int argc, char* argv[]) {
  bool is_data_ref_set = false;
  TString data_ref_path("");

  std::stringstream tempstream;
  tempstream << std::getenv("HOME") << "/plots";
  std::string output_directory_path(tempstream.str());

  int c;

  while ((c = getopt(argc, argv, "h")) != -1) {
    switch (c) {
    /*case 'r':
     data_ref_path = optarg;
     is_data_ref_set = true;
     break;*/
    case '?':
      if (optopt == 'r')
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

  int argoffset = optind;

  if (argc > 1) {
    std::vector<std::string> data_paths;
    for (int i = argoffset; i < argc; i++)
      data_paths.push_back(std::string(argv[i]));
    plotIPDistribution(data_paths, output_directory_path, "");
  }

  return 0;
}
