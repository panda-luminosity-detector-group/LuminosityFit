#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdPlotter.h"
#include "fit/PndLmdLumiFitResult.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"
#include "LumiFitStructs.h"
#include "PndLmdComparisonStructs.h"

#include "TGaxis.h"

#include <iostream>               // for std::cout
#include <map>
#include <vector>
#include <sstream>

using std::cout;
using std::endl;
using std::cerr;
using std::map;
using std::vector;

void saveGraphBundlesToFile(
    NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle &gb_mc,
    NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle &gb_mc_acc,
    NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle &gb_reco,
    std::string dependency_suffix) {

  TLatex suffix(0.0, 0.0, dependency_suffix.c_str());
  suffix.Write("suffix");

  gDirectory->mkdir("mc");
  gDirectory->cd("mc");
  std::cout << gb_mc.mean << std::endl;
  gDirectory->pwd();
  gb_mc.mean->Write("mean");
  gb_mc.sigma->Write("sigma");
  gb_mc.mean_individual_error->Write("lumifit_error");
  std::cout << "saved stuff" << std::endl;
  gDirectory->cd("..");
  gDirectory->mkdir("mc_acc");
  gDirectory->cd("mc_acc");
  gb_mc_acc.mean->Write("mean");
  gb_mc_acc.sigma->Write("sigma");
  gb_mc_acc.mean_individual_error->Write("lumifit_error");
  gDirectory->cd("..");
  gDirectory->mkdir("reco");
  gDirectory->cd("reco");
  gb_reco.mean->Write("mean");
  gb_reco.sigma->Write("sigma");
  gb_reco.mean_individual_error->Write("lumifit_error");
  std::cout << "saved stuff" << std::endl;
}

void determineLumiFitSystematics(std::vector<std::string> paths,
    const std::string &filter_string, const std::string &output_directory_path,
    std::string filename_suffix) {
  std::cout << "Generating lumi comparison plots for fit results....\n";

  // ================================ BEGIN CONFIG ================================ //
  // sets default pad margins etc that should be fine for most cases
  // you can fine tune it and overwrite the default values
  gStyle->SetPadRightMargin(0.165);
  gStyle->SetPadLeftMargin(0.125);
  gStyle->SetPadBottomMargin(0.127);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadColor(10);
  gStyle->SetCanvasColor(10);
  gStyle->SetStatColor(10);

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  // overwrite the default theta plot range if possible
  //plotter.setThetaPlotRange(0.5, 16.0);

  bool make_fit_range_dependency_plots(false);

  // ================================= END CONFIG ================================= //

  // A small helper class that helps to construct lmd data objects
  PndLmdDataFacade lmd_data_facade;

  LumiFit::PndLmdPlotter lmd_plotter;

  //  lmd_plotter.primary_dimension_plot_range.range_low = 0.5;
  //  lmd_plotter.primary_dimension_plot_range.range_high = 16.0;

  // get all data first
  std::vector<PndLmdFitDataBundle> all_data;

  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();

  for (unsigned int i = 0; i < paths.size(); i++) {
    // ------ get files -------------------------------------------------------
    std::vector<std::string> file_paths = lmd_data_facade.findFilesByName(
        paths[i], filter_string, "lmd_fitted_data.root");

    for (unsigned int j = 0; j < file_paths.size(); j++) {
      std::string fullpath = file_paths[j];
      TFile fdata(fullpath.c_str(), "READ");

      // read in data from a root file which will return a map of PndLmdAngularData objects
      std::vector<PndLmdFitDataBundle> data_vec =
          lmd_data_facade.getDataFromFile<PndLmdFitDataBundle>(fdata);

      // append all data objects to the end of the corresponding data map vectors
      all_data.insert(all_data.end(), data_vec.begin(), data_vec.end());
    }
  }

  for (unsigned int j = 0; j < all_data.size(); j++) {
    std::cout << "resolutions in object: "
        << all_data[j].getUsedResolutionsPool().size() << std::endl;
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

  // now get all data bundles that are full phi
  std::vector<PndLmdFitDataBundle> full_phi_vec;
  std::vector<PndLmdElasticDataBundle> full_phi_reco_data_vec;

  LumiFit::LmdDimensionOptions phi_slice_dim_opt;
  phi_slice_dim_opt.dimension_type = LumiFit::PHI;
  phi_slice_dim_opt.track_param_type = LumiFit::LMD;
  phi_slice_dim_opt.track_type = LumiFit::MC;

  LumiFit::Comparisons::NegatedSelectionDimensionFilter filter(
      phi_slice_dim_opt);

  for (auto const& fit_data_bundle : all_data) {

    std::vector<PndLmdElasticDataBundle> temp_full_phi_vec =
        lmd_data_facade.filterData(fit_data_bundle.getElasticDataBundles(),
            filter);

    if (temp_full_phi_vec.size() > 0)
      full_phi_vec.push_back(fit_data_bundle);
    full_phi_reco_data_vec.reserve(
        full_phi_reco_data_vec.size() + temp_full_phi_vec.size());
    for (unsigned int i = 0; i < temp_full_phi_vec.size(); ++i)
      full_phi_reco_data_vec.push_back(temp_full_phi_vec[i]);
  }

  // if we just need the luminosity values and do not have to build the models again
  // ---------- reco -- full phi stuff
  LumiFit::LmdDimensionOptions lmd_dim_opt;
  lmd_dim_opt.track_type = LumiFit::RECO;
  if (full_phi_reco_data_vec[0].getPrimaryDimension().dimension_options.dimension_type
      == LumiFit::THETA_X)
    lmd_dim_opt.dimension_type = LumiFit::THETA_X;

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter dim_filter(
      lmd_dim_opt);
  full_phi_reco_data_vec = lmd_data_facade.filterData(full_phi_reco_data_vec,
      dim_filter);

  TGraphAsymmErrors *graph =
      lmd_plotter.createBinningDependencyGraphBundle(full_phi_reco_data_vec,
          lmd_dim_opt);

  TCanvas c;
  graph->Draw("AP");
  c.SaveAs("binning.pdf");
  // ================================ END PLOTTING ================================ //
}

void displayInfo() {
// display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "list of directories to be scanned for elastic scattering data"
      << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-f [output filename suffix]" << std::endl;
  std::cout << "-o [output directory]" << std::endl;
  std::cout
      << "-s [filtering string, which has to appear in the full directory path for all found paths]"
      << std::endl;
}

int main(int argc, char* argv[]) {
  bool is_filename_suffix_set = false;
  std::string filename_suffix("fitresults");
  std::string filter_string("");

  std::stringstream tempstream;
  tempstream << std::getenv("HOME") << "/plots";
  std::string output_directory_path(tempstream.str());

  int c;

  while ((c = getopt(argc, argv, "hf:s:o:")) != -1) {
    switch (c) {
    case 'f':
      filename_suffix = optarg;
      is_filename_suffix_set = true;
      break;
    case 's':
      filter_string = optarg;
      break;
    case 'o':
      output_directory_path = optarg;
      break;
    case '?':
      if (optopt == 'f' || optopt == 's' || optopt == 'o')
        std::cerr << "Option -" << optopt << " requires an argument."
            << std::endl;
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
    determineLumiFitSystematics(paths, filter_string, output_directory_path,
        filename_suffix);
  }

  return 0;
}

