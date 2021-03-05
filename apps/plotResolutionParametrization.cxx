#include "PndLmdPlotter.h"
#include "data/PndLmdDataFacade.h"
#include "data/PndLmdHistogramData.h"

#include "TFile.h"
#include "TGaxis.h"
#include "TStyle.h"

#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

void plotResolutionParametrization2D(std::string input_file_dir,
                                     std::string input_ref_file_dir) {
  // create an instance of PndLmdResultPlotter the plotting helper class
  LumiFit::PndLmdPlotter plotter;
  // A small helper class that helps to construct lmddata objects
  PndLmdDataFacade data_facade;

  // ================================ BEGIN CONFIG
  // ================================ // PndLmdResultPlotter sets default pad
  // margins etc that should be fine for most cases you can fine tune it and
  // overwrite the default values

  gStyle->SetPadTopMargin(0.04);
  gStyle->SetPadRightMargin(0.19);
  gStyle->SetPadLeftMargin(0.121);
  gStyle->SetPadBottomMargin(0.145);

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  // gStyle->SetTitleX(0.3);
  // gStyle->SetTitleY();
  // gStyle->SetTitleW();
  // gStyle->SetTitleH();
  // ================================= END CONFIG
  // ================================= //

  // =============================== BEGIN PLOTTING
  // =============================== //

  std::stringstream input_filename_url;
  input_filename_url << input_file_dir << "/lmd_res_data.root";
  TFile *infile = new TFile(input_filename_url.str().c_str(), "READ");

  std::stringstream basepath;
  basepath << std::getenv("HOME") << "/plots";

  // read in data from a root file
  std::vector<PndLmdHistogramData> res_vec =
      data_facade.getDataFromFile<PndLmdHistogramData>(infile);

  // first lets create a booky of all resolutions with the fitted resolutions
  // cluster data into groups
  std::map<LumiFit::LmdDimension, std::vector<PndLmdHistogramData>>
      phi_slice_map;
  std::vector<PndLmdHistogramData> full_phi_vec;

  for (unsigned int j = 0; j < res_vec.size(); j++) {
    bool found_phi_selection = false;
    const std::set<LumiFit::LmdDimension> &selection_set =
        res_vec[j].getSelectorSet();
    std::set<LumiFit::LmdDimension>::const_iterator selection_set_it;
    for (selection_set_it = selection_set.begin();
         selection_set_it != selection_set.end(); selection_set_it++) {
      if (selection_set_it->dimension_options.dimension_type ==
          LumiFit::PHI_FIRST_LMD_PLANE) {
        phi_slice_map[*selection_set_it].push_back(res_vec[j]);
        found_phi_selection = true;
        break;
      }
    }
    if (!found_phi_selection) {
      full_phi_vec.push_back(res_vec[j]);
    }
  }

  std::stringstream filepath_base;
  filepath_base << basepath.str() << "/";
  filepath_base << "plab_" << full_phi_vec[0].getLabMomentum();

  std::stringstream filepath;
  /*NeatPlotting::Booky booky = plotter.makeResolutionFitResultBooky(
   full_phi_vec);
   filepath << filepath_base.str() << "/resolution.pdf";
   booky.createBooky(filepath.str());*/

  TCanvas c;
  NeatPlotting::PlotStyle mystyle;

  mystyle.x_axis_style.axis_text_style.text_size = 0.06;
  mystyle.y_axis_style.axis_text_style.text_size = 0.06;
  mystyle.z_axis_style.axis_text_style.text_size = 0.06;
  mystyle.x_axis_style.axis_title_text_offset = 1.1;
  mystyle.y_axis_style.axis_title_text_offset = 0.85;
  mystyle.z_axis_style.axis_title_text_offset = 0.95;

  for (unsigned int i = 0; i < full_phi_vec.size(); i++) {
    if (full_phi_vec[i].get2DHistogram()->GetEntries() > 4000) {
      const std::set<LumiFit::LmdDimension> &sel_dim =
          full_phi_vec[i].getSelectorSet();
      std::set<LumiFit::LmdDimension>::const_iterator sel_dim_iter;
      for (sel_dim_iter = sel_dim.begin(); sel_dim_iter != sel_dim.end();
           sel_dim_iter++) {
        std::cout << sel_dim_iter->dimension_options.dimension_type << " : "
                  << sel_dim_iter->dimension_range.getDimensionMean()
                  << std::endl;
      }
      NeatPlotting::PlotBundle plot_bundle =
          plotter.makeResolutionGraphBundle(full_phi_vec[i]);
      plot_bundle.plot_decoration.draw_labels = false;
      plot_bundle.drawOnCurrentPad(mystyle);
      filepath.str("");
      filepath << filepath_base.str() << "/resolution2d_example.pdf";
      c.SaveAs(filepath.str().c_str());
      break;
    }
  }
}

void plotResolutionParametrization1D(std::string input_file_dir,
                                     unsigned int parametrization_level,
                                     std::string input_ref_file_dir) {
  // create an instance of PndLmdResultPlotter the plotting helper class
  LumiFit::PndLmdPlotter plotter;
  // A small helper class that helps to construct lmddata objects
  PndLmdDataFacade data_facade;

  // ================================ BEGIN CONFIG
  // ================================ // PndLmdResultPlotter sets default pad
  // margins etc that should be fine for most cases you can fine tune it and
  // overwrite the default values

  gStyle->SetPadTopMargin(0.06);
  gStyle->SetPadBottomMargin(0.12);
  gStyle->SetPadLeftMargin(0.13);
  gStyle->SetPadRightMargin(0.07);

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  // gStyle->SetTitleX(0.3);
  // gStyle->SetTitleY();
  // gStyle->SetTitleW();
  // gStyle->SetTitleH();
  // ================================= END CONFIG
  // ================================= //

  // =============================== BEGIN PLOTTING
  // =============================== //

  std::stringstream input_filename_url;
  input_filename_url << input_file_dir << "/resolution_params_"
                     << parametrization_level << ".root";
  TFile *infile = new TFile(input_filename_url.str().c_str(), "READ");

  std::stringstream basepath;
  basepath << std::getenv("HOME") << "/plots";

  switch (parametrization_level) {
  case 0: {
    // read in data from a root file
    std::vector<PndLmdHistogramData> res_vec =
        data_facade.getDataFromFile<PndLmdHistogramData>(infile);

    // first lets create a booky of all resolutions with the fitted resolutions
    // cluster data into groups
    std::map<LumiFit::LmdDimension, std::vector<PndLmdHistogramData>>
        phi_slice_map;
    std::vector<PndLmdHistogramData> full_phi_vec;

    for (unsigned int j = 0; j < res_vec.size(); j++) {
      bool found_phi_selection = false;
      const std::set<LumiFit::LmdDimension> &selection_set =
          res_vec[j].getSelectorSet();
      std::set<LumiFit::LmdDimension>::const_iterator selection_set_it;
      for (selection_set_it = selection_set.begin();
           selection_set_it != selection_set.end(); selection_set_it++) {
        if (selection_set_it->dimension_options.dimension_type ==
            LumiFit::PHI_FIRST_LMD_PLANE) {
          phi_slice_map[*selection_set_it].push_back(res_vec[j]);
          found_phi_selection = true;
          break;
        }
      }
      if (!found_phi_selection) {
        full_phi_vec.push_back(res_vec[j]);
      }
    }

    std::stringstream filepath_base;
    filepath_base << basepath.str() << "/";
    filepath_base << "plab_" << full_phi_vec[0].getLabMomentum();

    std::map<LumiFit::LmdDimension, std::vector<PndLmdHistogramData>>::iterator
        phi_slice_iter;
    for (phi_slice_iter = phi_slice_map.begin();
         phi_slice_iter != phi_slice_map.end(); phi_slice_iter++) {

      std::stringstream title;
      title << "resolution_"
            << phi_slice_iter->first.dimension_range.getDimensionMean()
            << ".pdf";
      NeatPlotting::Booky booky =
          plotter.makeResolutionFitResultBooky(phi_slice_iter->second);

      std::stringstream filepath;
      filepath << filepath_base.str() << "/" << title.str();
      booky.createBooky(filepath.str());
    }

    NeatPlotting::Booky booky =
        plotter.makeResolutionFitResultBooky(full_phi_vec);
    std::stringstream filepath;
    filepath << filepath_base.str() << "/resolution.pdf";
    booky.createBooky(filepath.str());

    TCanvas c;
    NeatPlotting::PlotStyle mystyle;
    mystyle.x_axis_style.axis_text_style.text_size = 0.05;
    mystyle.y_axis_style.axis_text_style.text_size = 0.05;
    NeatPlotting::PlotBundle plot_bundle =
        plotter.makeResolutionGraphBundle(full_phi_vec[12]);
    // plot_bundle.plot_decoration.draw_labels = false;
    plot_bundle.plot_axis.x_axis_range.low = -0.002;
    plot_bundle.plot_axis.x_axis_range.high = 0.002;
    plot_bundle.plot_axis.x_axis_range.active = true;
    plot_bundle.drawOnCurrentPad(mystyle);
    filepath.str("");
    filepath << filepath_base.str() << "/resolution_slice_example.pdf";
    c.SaveAs(filepath.str().c_str());

    filepath.str("");
    filepath << filepath_base.str() << "/resolution_slices.root";
    TFile *resolution_slices = new TFile(filepath.str().c_str(), "RECREATE");
    for (unsigned int slice_index = 0; slice_index < full_phi_vec.size();
         slice_index++) {
      NeatPlotting::PlotBundle slice_plot_bundle =
          plotter.makeResolutionGraphBundle(full_phi_vec[slice_index]);

      const std::set<LumiFit::LmdDimension> &selector_set =
          full_phi_vec[slice_index].getSelectorSet();
      std::set<LumiFit::LmdDimension>::const_iterator selection_set_it;
      for (selection_set_it = selector_set.begin();
           selection_set_it != selector_set.end(); selection_set_it++) {
        if (selection_set_it->dimension_options.dimension_type ==
            LumiFit::THETA)
          break;
      }

      if (selection_set_it != selector_set.end()) {
        std::stringstream res_name;

        std::vector<NeatPlotting::DrawableDataObjectStylePair<TH1 *>> hists =
            slice_plot_bundle.getHistograms();
        if (hists.size() > 0) {
          res_name << "res1d_"
                   << 1000 * selection_set_it->dimension_range.getRangeLow()
                   << "-"
                   << 1000 * selection_set_it->dimension_range.getRangeHigh();

          slice_plot_bundle.getHistograms()[0].data_object->SetName(
              res_name.str().c_str());
          slice_plot_bundle.getHistograms()[0].data_object->Write();
          res_name.str("");
        }

        std::vector<NeatPlotting::DrawableDataObjectStylePair<TGraph *>>
            graphs = slice_plot_bundle.getGraphs();
        if (graphs.size() > 0) {
          res_name << "asymmgaus1d_"
                   << 1000 * selection_set_it->dimension_range.getRangeLow()
                   << "-"
                   << 1000 * selection_set_it->dimension_range.getRangeHigh();
          slice_plot_bundle.getGraphs()[0].data_object->SetName(
              res_name.str().c_str());
          slice_plot_bundle.getGraphs()[0].data_object->Write();
        }
      }
    }
    resolution_slices->Close();

    if (input_ref_file_dir != "") {

      std::stringstream ref_input_filename_url;
      ref_input_filename_url << input_ref_file_dir << "/resolution_params_"
                             << parametrization_level << ".root";
      TFile *ref_infile =
          new TFile(ref_input_filename_url.str().c_str(), "READ");

      // read in data from a root file
      std::vector<PndLmdHistogramData> ref_res_vec =
          data_facade.getDataFromFile<PndLmdHistogramData>(ref_infile);

      //	plotter.makeResolutionDifferencesBooky(res_vec, ref_res_vec);
    }
    break;
  }
  case 1: {
    // read in data from file
    PndLmdLumiHelper lmd_helper;
    std::vector<PndLmdLumiHelper::lmd_graph *> graphs =
        lmd_helper.getResolutionModelResultsFromFile(input_filename_url.str());

    std::map<TString,
             std::map<LumiFit::PndLmdFitModelOptions,
                      std::map<std::set<LumiFit::LmdDimension>,
                               std::vector<PndLmdLumiHelper::lmd_graph *>>>>
        dependency_type_sorted_graphs;

    for (unsigned int i = 0; i < graphs.size(); i++) {
      dependency_type_sorted_graphs[TString(graphs[i]->getDependencyString())]
                                   [graphs[i]
                                        ->fit_options->getFitModelOptions()]
                                   [graphs[i]->remaining_selections]
                                       .push_back(graphs[i]);
    }

    for (std::map<
             TString,
             std::map<LumiFit::PndLmdFitModelOptions,
                      std::map<std::set<LumiFit::LmdDimension>,
                               std::vector<PndLmdLumiHelper::lmd_graph *>>>>::
             const_iterator it = dependency_type_sorted_graphs.begin();
         it != dependency_type_sorted_graphs.end(); it++) {

      // sort graphs
      std::map<LumiFit::PndLmdFitModelOptions,
               std::map<std::set<LumiFit::LmdDimension>,
                        std::vector<PndLmdLumiHelper::lmd_graph *>>>
          sorted_graphs = it->second;

      // loop over different smearing models
      std::map<LumiFit::PndLmdFitModelOptions,
               std::map<std::set<LumiFit::LmdDimension>,
                        std::vector<PndLmdLumiHelper::lmd_graph *>>>::iterator
          resolution_model_sorted_graphs_iter;

      for (resolution_model_sorted_graphs_iter = sorted_graphs.begin();
           resolution_model_sorted_graphs_iter != sorted_graphs.end();
           resolution_model_sorted_graphs_iter++) {
        std::map<std::set<LumiFit::LmdDimension>,
                 std::vector<PndLmdLumiHelper::lmd_graph *>>
            selection_sorted_graphs =
                resolution_model_sorted_graphs_iter->second;

        std::map<std::set<LumiFit::LmdDimension>,
                 std::vector<PndLmdLumiHelper::lmd_graph *>>::iterator
            resolution_parametrization_graph_iter;
        for (resolution_parametrization_graph_iter =
                 selection_sorted_graphs.begin();
             resolution_parametrization_graph_iter !=
             selection_sorted_graphs.end();
             resolution_parametrization_graph_iter++) {

          // plot overview canvas
          NeatPlotting::Booky booky =
              plotter.createResolutionParameterizationOverviewPlot(
                  resolution_parametrization_graph_iter->second);

          std::stringstream filepath_base;
          filepath_base.precision(3);
          filepath_base << basepath.str() << "/";
          filepath_base
              << "plab_"
              << resolution_parametrization_graph_iter->second[0]->lab_momentum;

          boost::filesystem::create_directories(filepath_base.str());

          filepath_base << "/";
          if (resolution_model_sorted_graphs_iter->first.smearing_model ==
              LumiFit::GAUSSIAN)
            filepath_base << "gaus";
          else if (resolution_model_sorted_graphs_iter->first.smearing_model ==
                   LumiFit::ASYMMETRIC_GAUSSIAN)
            filepath_base << "asymm_gaus";
          else if (resolution_model_sorted_graphs_iter->first.smearing_model ==
                   LumiFit::DOUBLE_GAUSSIAN)
            filepath_base << "double_gaus";
          else
            filepath_base << "unknown_model";

          filepath_base << "_resolution_parameters_" << it->first;
          for (std::set<LumiFit::LmdDimension>::const_iterator dependency =
                   resolution_parametrization_graph_iter->first.begin();
               dependency != resolution_parametrization_graph_iter->first.end();
               dependency++) {
            filepath_base << "_" << *dependency;
          }

          std::stringstream filepath;
          filepath << filepath_base.str() << ".pdf";

          booky.createBooky(filepath.str());

          if (resolution_parametrization_graph_iter->second.size() > 1) {
            TCanvas c;
            NeatPlotting::PlotStyle ps;
            // for(unsigned int k = 0; k <
            // resolution_parametrization_graph_iter->second.size(); k++) {
            NeatPlotting::PlotBundle pb =
                plotter.createResolutionParameterizationPlot(
                    resolution_parametrization_graph_iter->second[2]);
            pb.plot_decoration.draw_labels = false;
            pb.drawOnCurrentPad(ps);

            filepath.str("");
            filepath << filepath_base.str() << "_"
                     << resolution_parametrization_graph_iter->second[2]
                            ->parameter_name_stack[0]
                            .second
                     << ".pdf";
            c.SaveAs(filepath.str().c_str());
          }

          // make comparison (difference) plots for different selections with
          // respect to some reference

          /*// so first determine the reference
           std::vector<PndLmdLumiHelper::lmd_graph*> reference =
           determineReferenceGraphs(selection_sorted_graphs);

           // now loop over the remaining and create comparison graphs
           std::map<std::set<LumiFit::LmdDimension>,
           std::vector<PndLmdLumiHelper::lmd_graph*> >::iterator
           selection_sorted_graphs_iter;

           for (selection_sorted_graphs_iter = selection_sorted_graphs.begin();
           selection_sorted_graphs_iter != selection_sorted_graphs.end();
           selection_sorted_graphs_iter++) {
           plotter.createResolutionParameterizationComparisonPlots(
           selection_sorted_graphs_iter->second, reference);
           }*/
        }
      }
    }
    break;
  }
  default: {
    std::cout << "Error: The requested parametrization level "
              << parametrization_level
              << " does not exist. The highest level is 3." << std::endl;

    break;
  }
  }

  // ================================ END PLOTTING
  // ================================ //
}

void displayInfo() {
  // display info
  cout << "Required arguments are: " << endl;
  cout << "-d [path to data]" << endl;
  cout << "Optional arguments are: " << endl;
  cout << "-l [parametrization level] (0 or 1)" << endl;
  cout << "-r [path to reference data]" << endl;
}

int main(int argc, char *argv[]) {
  bool is_data_set = false, is_data_ref_set = false, is_par_level_set = false;
  std::string data_path;
  std::string data_ref_path;
  int parametrization_level = 0;
  int c;

  while ((c = getopt(argc, argv, "hl:d:r:")) != -1) {
    switch (c) {
    case 'l':
      parametrization_level = atoi(optarg);
      is_par_level_set = true;
      break;
    case 'd':
      data_path = optarg;
      is_data_set = true;
      break;
    case 'r':
      data_ref_path = optarg;
      is_data_ref_set = true;
      break;
    case '?':
      if (optopt == 'd' || optopt == 'r' || optopt == 'l')
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

  if (is_data_set && is_par_level_set)
    plotResolutionParametrization1D(data_path, parametrization_level,
                                    data_ref_path);
  else if (is_data_set)
    plotResolutionParametrization2D(data_path, data_ref_path);
  else
    displayInfo();
  return 0;
}
