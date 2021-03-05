#include "LumiFitStructs.h"
#include "PndLmdComparisonStructs.h"
#include "data/PndLmdAcceptance.h"
#include "data/PndLmdAngularData.h"
#include "fit/PndLmdLumiFitResult.h"
#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdPlotter.h"

#include "TGaxis.h"

#include <iostream> // for std::cout
#include <map>
#include <sstream>
#include <vector>

#include "boost/property_tree/json_parser.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::vector;

void createSystematics(
    const std::vector<PndLmdElasticDataBundle> &prefiltered_data) {

  std::map<std::string, std::vector<std::pair<double, double>>> measured_values;

  PndLmdElasticDataBundle example_data_obj;

  for (auto const &prefiltered_data_obj : prefiltered_data) {
    example_data_obj = prefiltered_data_obj;
    std::cout << "generated lumi: "
              << prefiltered_data_obj.getReferenceLuminosity() << std::endl;

    const std::map<PndLmdFitOptions, std::vector<ModelFitResult>> &fit_results =
        prefiltered_data_obj.getFitResults();

    for (auto const &fit_result_pair : fit_results) {
      if (fit_result_pair.first.getModelOptionsPropertyTree().get<bool>(
              "divergence_smearing_active") == true) {
        if (fit_result_pair.second.size() > 0) {
          ModelFitResult fit_result = fit_result_pair.second[0];
          if (fit_result.getFitStatus() != 0)
            continue;

          auto fit_params = fit_result.getFitParameters();
          for (auto fit_param : fit_params) {
            measured_values[fit_param.name.second].push_back(
                std::make_pair(fit_param.value, fit_param.error));
          }
        }
      }
    }
  }

  boost::property_tree::ptree all_scenario_tree;

  for (auto const &fit_param_values : measured_values) {
    boost::property_tree::ptree measured_values_tree;

    boost::property_tree::ptree value_tree;
    for (auto const &value : fit_param_values.second) {
      boost::property_tree::ptree tempptree;
      tempptree.put("", value.first);
      value_tree.push_back(std::make_pair("", tempptree));
    }

    measured_values_tree.add_child("values", value_tree);

    boost::property_tree::ptree error_tree;
    for (auto const &value : fit_param_values.second) {
      boost::property_tree::ptree tempptree;
      tempptree.put("", value.second);
      error_tree.push_back(std::make_pair("", tempptree));
    }
    measured_values_tree.add_child("errors", error_tree);

    all_scenario_tree.add_child(fit_param_values.first, measured_values_tree);
  }

  boost::property_tree::ptree gen_values;
  // TODO fix this: example_data_obj.getSimulationParametersPropertyTree());
  gen_values.put("lumi", example_data_obj.getReferenceLuminosity());
  all_scenario_tree.add_child("generated", gen_values);

  std::stringstream filename;
  filename << "/home/pflueger/plots/lumi_systematics.json";
  write_json(filename.str(), all_scenario_tree);
}

void determineLumiFitSystematics(std::vector<std::string> paths,
                                 const std::string &filter_string,
                                 const std::string &output_directory_path,
                                 std::string filename_suffix) {
  std::cout << "Generating lumi comparison plots for fit results....\n";

  // ================================ BEGIN CONFIG
  // ================================ // sets default pad margins etc that
  // should be fine for most cases you can fine tune it and overwrite the
  // default values
  gStyle->SetPadRightMargin(0.07);
  gStyle->SetPadLeftMargin(0.138);
  gStyle->SetPadBottomMargin(0.127);
  gStyle->SetPadTopMargin(0.03);
  gStyle->SetPadColor(10);
  gStyle->SetCanvasColor(10);
  gStyle->SetStatColor(10);

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  // overwrite the default theta plot range if possible
  // plotter.setThetaPlotRange(0.5, 16.0);

  bool make_fit_range_dependency_plots(false);

  // ================================= END CONFIG
  // ================================= //

  // A small helper class that helps to construct lmd data objects
  PndLmdDataFacade lmd_data_facade;

  LumiFit::PndLmdPlotter lmd_plotter;

  //  lmd_plotter.primary_dimension_plot_range.range_low = 0.5;
  //  lmd_plotter.primary_dimension_plot_range.range_high = 16.0;

  // get all data first
  std::vector<PndLmdFitDataBundle> all_data;

  PndLmdRuntimeConfiguration &lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();

  for (unsigned int i = 0; i < paths.size(); i++) {
    // ------ get files -------------------------------------------------------
    std::vector<std::string> file_paths = lmd_data_facade.findFilesByName(
        paths[i], filter_string, "lmd_fitted_data.root");

    for (unsigned int j = 0; j < file_paths.size(); j++) {
      std::string fullpath = file_paths[j];
      TFile fdata(fullpath.c_str(), "READ");

      // read in data from a root file which will return a map of
      // PndLmdAngularData objects
      std::vector<PndLmdFitDataBundle> data_vec =
          lmd_data_facade.getDataFromFile<PndLmdFitDataBundle>(fdata);

      // append all data objects to the end of the corresponding data map
      // vectors
      all_data.insert(all_data.end(), data_vec.begin(), data_vec.end());
    }
  }

  for (unsigned int j = 0; j < all_data.size(); j++) {
    std::cout << "resolutions in object: "
              << all_data[j].getUsedResolutionsPool().size() << std::endl;
  }

  // =============================== BEGIN PLOTTING
  // =============================== //

  std::stringstream basepath;
  basepath << output_directory_path;

  if (!boost::filesystem::exists(output_directory_path)) {
    std::cout << "The output directory path you specified does not exist! "
                 "Please make sure you are pointing to an existing directory."
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

  for (auto const &fit_data_bundle : all_data) {

    std::vector<PndLmdElasticDataBundle> temp_full_phi_vec =
        lmd_data_facade.filterData(fit_data_bundle.getElasticDataBundles(),
                                   filter);

    if (temp_full_phi_vec.size() > 0)
      full_phi_vec.push_back(fit_data_bundle);
    full_phi_reco_data_vec.reserve(full_phi_reco_data_vec.size() +
                                   temp_full_phi_vec.size());
    for (unsigned int i = 0; i < temp_full_phi_vec.size(); ++i)
      full_phi_reco_data_vec.push_back(temp_full_phi_vec[i]);
  }

  // if we just need the luminosity values and do not have to build the models
  // again

  NeatPlotting::PlotBundle bundle;
  // plot_style.palette_color_style = 1;
  bundle.plot_axis.x_axis_title = "#theta_{x} & #theta_{y} binning";
  bundle.plot_axis.y_axis_title = "#frac{L_{fit}-L_{ref}}{L_{ref}} /%";

  NeatPlotting::DataObjectStyle style;
  style.draw_option = "PE";

  NeatPlotting::PlotStyle plot_style;
  plot_style.y_axis_style.axis_title_text_offset = 1.22;

  LumiFit::LmdDimensionOptions lmd_dim_opt;
  lmd_dim_opt.track_type = LumiFit::MC_ACC;
  if (full_phi_reco_data_vec[0]
          .getPrimaryDimension()
          .dimension_options.dimension_type == LumiFit::THETA_X)
    lmd_dim_opt.dimension_type = LumiFit::THETA_X;

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter dim_filter(
      lmd_dim_opt);
  auto acc_full_phi_data_vec =
      lmd_data_facade.filterData(full_phi_reco_data_vec, dim_filter);

  TFile output_rootfile("depend.root", "RECREATE");

  if (acc_full_phi_data_vec.size() > 0) {
    TGraphAsymmErrors *graph = lmd_plotter.createBinningDependencyGraphBundle(
        acc_full_phi_data_vec, lmd_dim_opt);

    output_rootfile.cd();
    graph->Write("mc_acc_vs_binning_graph");

    style.marker_style.marker_style = 24;
    style.marker_style.marker_size = 1.3;
    bundle.addGraph(graph, style);
  }

  lmd_dim_opt.track_type = LumiFit::RECO;
  if (full_phi_reco_data_vec[0]
          .getPrimaryDimension()
          .dimension_options.dimension_type == LumiFit::THETA_X)
    lmd_dim_opt.dimension_type = LumiFit::THETA_X;

  dim_filter =
      LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter(lmd_dim_opt);
  full_phi_reco_data_vec =
      lmd_data_facade.filterData(full_phi_reco_data_vec, dim_filter);

  if (full_phi_reco_data_vec.size() > 0) {
    // createSystematics(full_phi_reco_data_vec);

    TGraphAsymmErrors *graph = lmd_plotter.createBinningDependencyGraphBundle(
        full_phi_reco_data_vec, lmd_dim_opt);

    output_rootfile.cd();
    graph->Write("reco_vs_binning_graph");

    style.marker_style.marker_color = 9;
    style.marker_style.marker_style = 32;
    style.marker_style.marker_size = 1.3;
    bundle.addGraph(graph, style);
  }

  output_rootfile.Write();
  output_rootfile.Close();
  // ================================ END PLOTTING
  // ================================ //
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "list of directories to be scanned for elastic scattering data"
            << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-f [output filename suffix]" << std::endl;
  std::cout << "-o [output directory]" << std::endl;
  std::cout << "-s [filtering string, which has to appear in the full "
               "directory path for all found paths]"
            << std::endl;
}

int main(int argc, char *argv[]) {
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
