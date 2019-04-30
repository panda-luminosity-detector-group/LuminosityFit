#include "ui/PndLmdPlotter.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"
#include "ui/PndLmdDataFacade.h"
#include "data/PndLmdFitDataBundle.h"

#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <sstream>

#include "boost/filesystem.hpp"
#include "boost/regex.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <boost/algorithm/string.hpp>

void extractLuminosityResults(std::vector<std::string> paths, const std::string &filter_string) {
  std::cout << "Generating lumi plots for fit results....\n";

  PndLmdDataFacade lmd_data_facade;

  // get all data first
  std::map<std::string, std::vector<PndLmdFitDataBundle> > all_data;

  std::vector<std::string> ip_files;

  for (unsigned int i = 0; i < paths.size(); i++) {
    // ------ get files -------------------------------------------------------
    std::vector<std::string> file_paths = lmd_data_facade.findFilesByName(paths[i], filter_string,
        "lmd_fitted_data.root");
    std::cout << "found " << file_paths.size() << " file paths!\n";
    for (auto file_path : file_paths) {
      std::string fullpath = file_path;
      TFile fdata(fullpath.c_str(), "READ");

      // read in data from a root file which will return a map of PndLmdAngularData objects
      all_data[file_path] = lmd_data_facade.getDataFromFile<PndLmdFitDataBundle>(fdata);
    }
  }

  // now get only the reco data bundle that are full phi
  std::map<std::string, PndLmdElasticDataBundle> full_phi_reco_data_vec;

  LumiFit::LmdDimensionOptions phi_slice_dim_opt;
  phi_slice_dim_opt.dimension_type = LumiFit::PHI;
  phi_slice_dim_opt.track_param_type = LumiFit::LMD;
  phi_slice_dim_opt.track_type = LumiFit::MC;

  LumiFit::LmdDimensionOptions lmd_dim_opt;
  lmd_dim_opt.dimension_type = LumiFit::THETA_X;

  LumiFit::Comparisons::NegatedSelectionDimensionFilter filter(phi_slice_dim_opt);
  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter dim_filter(lmd_dim_opt);

  for (auto const& datavec : all_data) {
    for (auto const& x : datavec.second) {
      std::vector<PndLmdElasticDataBundle> temp_full_phi_vec = lmd_data_facade.filterData(
          x.getElasticDataBundles(), filter);

      temp_full_phi_vec = lmd_data_facade.filterData(temp_full_phi_vec, dim_filter);

      if (temp_full_phi_vec.size() == 1)
        full_phi_reco_data_vec[datavec.first] = temp_full_phi_vec[0];
    }
  }

  LumiFit::PndLmdPlotter lmd_plotter;
  for (auto const &scenario : full_phi_reco_data_vec) {
    if (scenario.second.getFitResults().size() > 0
        && scenario.second.getSelectorSet().size() == 0) {
      PndLmdLumiFitResult fit_result;
      for (auto const& fit_res_pair : scenario.second.getFitResults()) {
        if (fit_res_pair.second.size() > 0) {
          bool div_smeared = fit_res_pair.first.getModelOptionsPropertyTree().get<bool>(
              "divergence_smearing_active");
          if (div_smeared) {
            if (fit_res_pair.second[0].getFitStatus() == 0) {
              fit_result.setModelFitResult(fit_res_pair.second[0]);
              break;
            }
          } else {
            fit_result.setModelFitResult(fit_res_pair.second[0]);
          }
        }
      }

      // try to open ip measurement file... this is quite dirty but no time to do it nice...
      boost::property_tree::ptree lumi_values;

      lumi_values.put("measured_lumi", fit_result.getLuminosity());
      lumi_values.put("measured_lumi_error", fit_result.getLuminosityError());
      lumi_values.put("generated_lumi", scenario.second.getReferenceLuminosity());
      std::pair<double, double> lumi = lmd_plotter.calulateRelDiff(fit_result.getLuminosity(),
          fit_result.getLuminosityError(), scenario.second.getReferenceLuminosity());
      lumi_values.put("relative_deviation_in_percent", lumi.first);
      lumi_values.put("relative_deviation_error_in_percent", lumi.second);

      std::stringstream filename;
      filename << boost::filesystem::path(scenario.first).parent_path().string();

      std::cout << filename.str() << ":\n====================\n";
      std::cout << "Luminosity Fit Result:\n";
      std::cout << "measured luminosity:" << fit_result.getLuminosity() << "\n";
      std::cout << "measured luminosity error:" << fit_result.getLuminosityError() << "\n";
      std::cout << "generated luminosity:" << scenario.second.getReferenceLuminosity() << "\n";
      std::cout << "relative deviation (%):" << lumi.first << "\n";
      std::cout << "relative deviation error (%):" << lumi.second << "\n";

      filename << "/lumi-values.json";
      write_json(filename.str(), lumi_values);
    }
  }
}

void displayInfo() {
// display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "list of directories to be scanned for fitted data" << std::endl;
  std::cout
      << "-s [filtering string, which has to appear in the full directory path for all found paths]"
      << std::endl;
}

int main(int argc, char* argv[]) {
  std::string filter_string("");
  int c;
  while ((c = getopt(argc, argv, "hs:")) != -1) {
    switch (c) {
      case 's':
        filter_string = optarg;
        break;
      case '?':
        if (optopt == 's')
          std::cerr << "Option -" << optopt << " requires an argument." << std::endl;
        else if (isprint(optopt))
          std::cerr << "Unknown option -" << optopt << "." << std::endl;
        else
          std::cerr << "Unknown option character" << optopt << "." << std::endl;
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
    std::vector<std::string> paths;
    for (unsigned int i = argoffset; i < argc; i++) {
      paths.push_back(std::string(argv[i]));
    }
    extractLuminosityResults(paths, filter_string);
  }

  return 0;
}
