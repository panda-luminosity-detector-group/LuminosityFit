#include "PndLmdPlotter.h"
#include "data/PndLmdAcceptance.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdDataFacade.h"

#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>

#include "boost/filesystem.hpp"

#include "TGaxis.h"

void makeTalkPlots(std::string path) {
  std::cout << "Generating lumi plots for fit results....\n";

  // A small helper class that helps to construct lmd data objects
  PndLmdDataFacade lmd_data_facade;

  LumiFit::PndLmdPlotter lmd_plotter;

  // ================================ BEGIN CONFIG
  // ================================ // PndLmdResultPlotter sets default pad
  // margins etc that should be fine for most cases you can fine tune it and
  // overwrite the default values
  gStyle->SetPadRightMargin(0.125);
  gStyle->SetPadLeftMargin(0.11);
  gStyle->SetPadBottomMargin(0.115);
  gStyle->SetPadColor(10);
  gStyle->SetCanvasColor(10);
  gStyle->SetStatColor(10);

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  // ================================= END CONFIG
  // ================================= //

  // create and fill data map first of all
  std::map<LumiFit::LmdSimIPParameters, std::vector<PndLmdAngularData>>
      data_map;

  // ------ get files -------------------------------------------------------
  std::vector<std::string> file_paths = lmd_data_facade.findFilesByName(
      path, "merge_data", "lmd_fitted_data.root");

  for (unsigned int j = 0; j < file_paths.size(); j++) {
    std::string fullpath = file_paths[j];
    TFile *fdata = new TFile(fullpath.c_str(), "READ");

    // get simulation ip distribution properties
    /*	LumiFit::LmdSimIPParameters true_ip_values =
                            lmd_data_facade.readSimulationIPParameters(
                                            (boost::filesystem::path(path).branch_path().branch_path().branch_path()).string());

            // read in data from a root file which will return a map of
       PndLmdAngularData objects std::vector<PndLmdAngularData> data_vec =
       lmd_data_facade.getDataFromFile< PndLmdAngularData>(fdata);

            // append all data objects to the end of the corresponding data map
       vectors data_map[true_ip_values].insert(data_map[true_ip_values].end(),
                            data_vec.begin(), data_vec.end());*/
  }

  // =============================== BEGIN PLOTTING
  // =============================== //

  std::stringstream basepath;
  basepath << std::getenv("HOME") << "/plots";

  // now loop over this map and create plots for each ip parameter setting
  std::map<LumiFit::LmdSimIPParameters,
           std::vector<PndLmdAngularData>>::iterator data_map_iter;
  for (data_map_iter = data_map.begin(); data_map_iter != data_map.end();
       data_map_iter++) {
    std::vector<PndLmdAngularData> data_vec = data_map_iter->second;
    // group data into same selections
    // and make sure that one selection is of the phi slice type
    std::map<LumiFit::LmdDimension, std::vector<PndLmdAngularData>>
        phi_slice_map;

    std::vector<PndLmdAngularData> full_phi_vec;

    for (unsigned int j = 0; j < data_vec.size(); j++) {
      bool found_phi_selection = false;
      const std::set<LumiFit::LmdDimension> &selection_set =
          data_vec[j].getSelectorSet();
      std::set<LumiFit::LmdDimension>::const_iterator selection_set_it;
      for (selection_set_it = selection_set.begin();
           selection_set_it != selection_set.end(); selection_set_it++) {
        if (selection_set_it->dimension_options.dimension_type ==
            LumiFit::PHI_FIRST_LMD_PLANE) {
          phi_slice_map[*selection_set_it].push_back(data_vec[j]);
          found_phi_selection = true;
          break;
        }
      }
      if (!found_phi_selection) {
        full_phi_vec.push_back(data_vec[j]);
      }
    }

    if (full_phi_vec.size() > 0) {
      std::stringstream filepath_base;
      filepath_base << basepath.str() << "/";
      filepath_base << "plab_" << full_phi_vec[0].getLabMomentum() << "/";
      filepath_base << data_map_iter->first.getLabel();

      boost::filesystem::create_directories(filepath_base.str());

      // ---------- reco -- full phi stuff
      LumiFit::LmdDimensionOptions lmd_dim_opt;
      lmd_dim_opt.track_type = LumiFit::RECO;

      LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(
          lmd_dim_opt);
      std::vector<PndLmdAngularData> full_phi_reco_data_vec =
          lmd_data_facade.filterData(full_phi_vec, filter);

      double plot_range_low = 0.0015;
      double plot_range_high = 0.01;

      std::stringstream filepath;
      TCanvas c("", "", 1000, 700);

      // reco single plot
      for (unsigned int reco_data_obj_index = 0;
           reco_data_obj_index < full_phi_reco_data_vec.size();
           reco_data_obj_index++) {

        // in case we filtered some data... skip that data object
        if (full_phi_reco_data_vec[reco_data_obj_index]
                .getSelectorSet()
                .size() > 0)
          continue;

        const map<PndLmdFitOptions, ModelFitResult> &fit_results =
            full_phi_reco_data_vec[reco_data_obj_index].getFitResults();

        NeatPlotting::PlotStyle single_plot_style;
        single_plot_style.x_axis_style.axis_text_style.text_size = 0.05;
        single_plot_style.y_axis_style.axis_text_style.text_size = 0.05;

        map<PndLmdFitOptions, ModelFitResult>::const_iterator fit_result;
        for (fit_result = fit_results.begin(); fit_result != fit_results.end();
             fit_result++) {
          c.Clear();

          std::cout << "fit result options: " << fit_result->first << std::endl;

          if (fit_result->first.getFitModelOptions()
                  .resolution_smearing_active) {
            NeatPlotting::PlotBundle reco_plot_bundle =
                lmd_plotter.makeGraphBundle(
                    full_phi_reco_data_vec[reco_data_obj_index],
                    fit_result->first);
            reco_plot_bundle.plot_axis.x_axis_range.low = plot_range_low;
            reco_plot_bundle.plot_axis.x_axis_range.high = plot_range_high;
            reco_plot_bundle.plot_axis.x_axis_range.active = true;
            reco_plot_bundle.plot_decoration.draw_labels = false;
            reco_plot_bundle.drawOnCurrentPad(single_plot_style);
            filepath.str("");
            filepath << filepath_base.str() << "/talk_plot_fit_result_reco.pdf";
            c.SaveAs(filepath.str().c_str());

            PndLmdFitOptions temp;
            NeatPlotting::PlotBundle data_only_reco_plot_bundle =
                lmd_plotter.makeGraphBundle(
                    full_phi_reco_data_vec[reco_data_obj_index], temp, false);
            data_only_reco_plot_bundle.plot_axis.x_axis_range.low =
                plot_range_low;
            data_only_reco_plot_bundle.plot_axis.x_axis_range.high =
                plot_range_high;
            data_only_reco_plot_bundle.plot_axis.x_axis_range.active = true;
            data_only_reco_plot_bundle.plot_decoration.draw_labels = false;
            data_only_reco_plot_bundle.drawOnCurrentPad(single_plot_style);
            filepath.str("");
            filepath << filepath_base.str()
                     << "/talk_plot_fit_result_reco_data_only.pdf";
            c.SaveAs(filepath.str().c_str());

            NeatPlotting::PlotBundle acc_bundle_1d =
                lmd_plotter.makeAcceptanceBundle1D(full_phi_reco_data_vec[0],
                                                   fit_results.begin()->first);
            acc_bundle_1d.plot_axis.x_axis_range.low = plot_range_low;
            acc_bundle_1d.plot_axis.x_axis_range.high = plot_range_high;
            acc_bundle_1d.plot_axis.x_axis_range.active = true;
            acc_bundle_1d.drawOnCurrentPad(single_plot_style);
            filepath.str("");
            filepath << filepath_base.str() << "/talk_plot_acceptance1d.pdf";
            c.SaveAs(filepath.str().c_str());

            NeatPlotting::PlotBundle acc_bundle_2d =
                lmd_plotter.makeAcceptanceBundle2D(full_phi_reco_data_vec[0],
                                                   fit_results.begin()->first);
            acc_bundle_2d.plot_axis.x_axis_range.active = true;
            // acc_bundle_2d.plot_axis.x_axis_range.low = 0.002;
            // acc_bundle_2d.plot_axis.x_axis_range.high = 0.01;
            acc_bundle_2d.drawOnCurrentPad(single_plot_style);
            filepath.str("");
            filepath << filepath_base.str() << "/talk_plot_acceptance2d.png";
            c.SaveAs(filepath.str().c_str());
          } else if (fit_result->first.getFitModelOptions()
                         .acceptance_correction_active &&
                     !fit_result->first.getFitModelOptions()
                          .resolution_smearing_active) {
            NeatPlotting::PlotBundle mc_acc_plot_bundle =
                lmd_plotter.makeGraphBundle(
                    full_phi_reco_data_vec[reco_data_obj_index],
                    fit_result->first);
            mc_acc_plot_bundle.plot_decoration.draw_labels = false;
            mc_acc_plot_bundle.plot_axis.x_axis_range.low = plot_range_low;
            mc_acc_plot_bundle.plot_axis.x_axis_range.high = plot_range_high;
            mc_acc_plot_bundle.plot_axis.x_axis_range.active = true;
            mc_acc_plot_bundle.drawOnCurrentPad(single_plot_style);
            filepath.str("");
            filepath << filepath_base.str()
                     << "/talk_plot_fit_result_mc_acc.pdf";
            c.SaveAs(filepath.str().c_str());
          }
          std::pair<double, double> lumi = lmd_plotter.calulateRelDiff(
              lmd_plotter.getLuminosity(fit_result->second),
              lmd_plotter.getLuminosityError(fit_result->second),
              full_phi_reco_data_vec[reco_data_obj_index]
                  .getReferenceLuminosity());
          std::cout << lumi.first << " +- " << lumi.second << std::endl;
        }
      }

      lmd_dim_opt.track_type = LumiFit::MC;

      LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter_mc(
          lmd_dim_opt);
      std::vector<PndLmdAngularData> full_phi_mc_data_vec =
          lmd_data_facade.filterData(full_phi_vec, filter_mc);

      if (full_phi_mc_data_vec.size() == 1) {
        const map<PndLmdFitOptions, PndLmdLumiFitResult> &fit_results =
            full_phi_mc_data_vec[0].getFitResults();
        if (fit_results.size() > 0) {
          NeatPlotting::PlotStyle single_plot_style;

          c.Clear();
          lmd_plotter.primary_dimension_plot_range.is_active = true;
          lmd_plotter.primary_dimension_plot_range.range_low = plot_range_low;
          lmd_plotter.primary_dimension_plot_range.range_high = plot_range_high;
          NeatPlotting::PlotBundle mc_plot_bundle = lmd_plotter.makeGraphBundle(
              full_phi_mc_data_vec[0], fit_results.begin()->first);
          mc_plot_bundle.plot_decoration.draw_labels = false;
          mc_plot_bundle.plot_axis.x_axis_range.low = plot_range_low;
          mc_plot_bundle.plot_axis.x_axis_range.high = plot_range_high;
          mc_plot_bundle.plot_axis.x_axis_range.active = true;
          mc_plot_bundle.drawOnCurrentPad(single_plot_style);
          filepath.str("");
          filepath << filepath_base.str() << "/talk_plot_fit_result_mc.pdf";
          c.SaveAs(filepath.str().c_str());
        }
      }
    }
    // ================================ END PLOTTING
    // ================================ //
  }
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "path to fitted angular data" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    makeTalkPlots(std::string(argv[1]));
  } else
    displayInfo();

  return 0;
}
