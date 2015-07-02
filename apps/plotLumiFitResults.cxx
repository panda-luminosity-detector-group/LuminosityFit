/*
 * This is an app that produces plots from the luminosity fitting procedure
 * General information about the individual classes of the LmdFit framework can be
 * found in the doxygen manual
 * Run it like this:
 * 
 * ./makeLumiFitPlots dir_to_elastic_scattering_data
 *
 * note: the dir_to_elastic_scattering_data should contain the fit_result.root root
 * file that contains the data and fit result on which fit were performed...
 *
 * The usage of this macro is straight forward and uses primarily the
 * PndLmdResultPlotter class (which is a helper class for creating plots on luminosity
 * fits).
 * -First you read in a PndLmdAngularData object that was saved by the runLumi6Fit.C macro
 * -Then you create so graph bundles that contain all the information of the fit
 *  results and data in form of ROOT objects.
 * -Finally you can pass these bundles to functions of the plotter and generate
 *  different type of plots
 */

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

#include "TGaxis.h"

void plotLumiFitResults(std::vector<std::string> paths,
		const std::string &filter_string, const std::string &output_directory_path,
		TString filename_suffix) {
	std::cout << "Generating lumi plots for fit results....\n";

// ================================ BEGIN CONFIG ================================ //
// PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
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

	bool make_1d_plots(false);
	bool make_2d_plots(true);
	bool make_single_scenario_bookies(true);
	bool make_offset_overview_plots(false);
	bool make_tilt_overview_plots(false);
	bool make_fit_range_dependency_plots(false);

// ================================= END CONFIG ================================= //

// A small helper class that helps to construct lmd data objects
	PndLmdDataFacade lmd_data_facade;

	LumiFit::PndLmdPlotter lmd_plotter;

//	lmd_plotter.primary_dimension_plot_range.range_low = 0.5;
//	lmd_plotter.primary_dimension_plot_range.range_high = 16.0;

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

	std::vector<PndLmdFitDataBundle>::iterator all_data_iter;
	for (all_data_iter = all_data.begin(); all_data_iter != all_data.end();
			++all_data_iter) {

		std::vector<PndLmdElasticDataBundle> temp_full_phi_vec =
				lmd_data_facade.filterData(all_data_iter->getElasticDataBundles(),
						filter);

		if (temp_full_phi_vec.size() > 0)
			full_phi_vec.push_back(*all_data_iter);
		full_phi_reco_data_vec.reserve(
				full_phi_reco_data_vec.size() + temp_full_phi_vec.size());
		for (unsigned int i = 0; i < temp_full_phi_vec.size(); ++i)
			full_phi_reco_data_vec.push_back(temp_full_phi_vec[i]);
	}

	// if we just need the luminosity values and do not have to build the models again
	// ---------- reco -- full phi stuff
	LumiFit::LmdDimensionOptions lmd_dim_opt;
	if (full_phi_reco_data_vec[0].getPrimaryDimension().dimension_options.dimension_type
			== LumiFit::THETA_X)
		lmd_dim_opt.dimension_type = LumiFit::THETA_X;

	LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter dim_filter(
			lmd_dim_opt);
	full_phi_reco_data_vec = lmd_data_facade.filterData(full_phi_reco_data_vec,
			dim_filter);

	if (make_offset_overview_plots && full_phi_reco_data_vec.size() > 1) {
		std::stringstream filename;
		filename << basepath.str() << "/";
		filename << "plab_" << full_phi_reco_data_vec.begin()->getLabMomentum()
				<< "/lumifit_result_ip_overview.pdf";

		NeatPlotting::PlotBundle bundle = lmd_plotter.makeXYOverviewHistogram(
				full_phi_reco_data_vec);

		TCanvas c;
		NeatPlotting::PlotStyle plot_style;
		plot_style.palette_color_style = 0;
		plot_style.z_axis_style.log_scale = true;
		bundle.drawOnCurrentPad(plot_style);
		c.SaveAs(filename.str().c_str());
	}
	if (make_tilt_overview_plots && full_phi_reco_data_vec.size() > 1) {
		std::stringstream filename;
		filename << basepath.str() << "/";
		filename << "plab_" << full_phi_reco_data_vec.begin()->getLabMomentum()
				<< "/lumifit_result_tilt_overview.pdf";

		std::cout << "build overview histogram with "
				<< full_phi_reco_data_vec.size() << " values!" << std::endl;

		std::vector<PndLmdElasticDataBundle> filtered_reco_data_objects;

		// filter reco_data_ip_map for tilts below some threshold
		double threshold = 0.0007;
		std::vector<PndLmdElasticDataBundle>::iterator reco_data_object_iter;
		for (reco_data_object_iter = full_phi_reco_data_vec.begin();
				reco_data_object_iter != full_phi_reco_data_vec.end();
				reco_data_object_iter++) {
			if (reco_data_object_iter->getSimulationParametersPropertyTree().get<
					double>("beam_tilt_x") < threshold
					&& reco_data_object_iter->getSimulationParametersPropertyTree().get<
							double>("beam_tilt_y") < threshold) {
				filtered_reco_data_objects.push_back(*reco_data_object_iter);
			}
		}

		NeatPlotting::PlotBundle bundle = lmd_plotter.makeTiltXYOverviewHistogram(
				filtered_reco_data_objects, 2);
		bundle.plot_axis.z_axis_range.active = true;
		bundle.plot_axis.z_axis_range.low = 0.0;
		bundle.plot_axis.z_axis_range.high = 0.1;

		TCanvas c;
		NeatPlotting::PlotStyle plot_style;
		plot_style.palette_color_style = 0;
		plot_style.z_axis_style.axis_title_text_offset = 1.4;
		plot_style.z_axis_style.log_scale = false;
		bundle.drawOnCurrentPad(plot_style);
		c.SaveAs(filename.str().c_str());
	}

	// now the stuff that really need to generate the model
	for (unsigned int fit_data_bundle_index = 0;
			fit_data_bundle_index < full_phi_vec.size(); ++fit_data_bundle_index) {
		std::stringstream filepath_base;
		filepath_base << basepath.str() << "/";
		filepath_base << "plab_"
				<< full_phi_vec[fit_data_bundle_index].getElasticDataBundles()[0].getLabMomentum()
				<< "/";
		filepath_base
				<< lmd_plotter.makeDirName(
						full_phi_vec[fit_data_bundle_index].getElasticDataBundles()[0]);

		boost::filesystem::create_directories(filepath_base.str());

		std::stringstream filepath;
		TCanvas c("", "", 1000, 700);

		lmd_plotter.setCurrentFitDataBundle(full_phi_vec[fit_data_bundle_index]);

		/*if (make_fit_range_dependency_plots) {
		 NeatPlotting::PlotBundle plot_bundle =
		 lmd_plotter.createLowerFitRangeDependencyPlotBundle(
		 full_phi_reco_data_vec, lmd_dim_opt);

		 filepath << filepath_base.str() << "/reco_lower_fit_range_dependency.pdf";

		 NeatPlotting::PlotStyle plot_style;
		 plot_bundle.drawOnCurrentPad(plot_style);
		 c.SaveAs(filepath.str().c_str());
		 }*/

		// create a vector of graph bundles (one entry for each fit option)
		if (make_single_scenario_bookies) {
			/*if (make_1d_plots) {
			 filepath.str("");
			 filepath << filepath_base.str() << "/fit_result_overview_booky.pdf";

			 NeatPlotting::Booky booky = lmd_plotter.makeLumiFitResultOverviewBooky(
			 full_phi_vec);
			 booky.createBooky(filepath.str());
			 }*/
			if (make_2d_plots) {
				filepath.str("");
				filepath << filepath_base.str() << "/fit_result_overview_booky_2d.pdf";

				NeatPlotting::Booky booky2 = lmd_plotter.create2DFitResultPlots(
						full_phi_vec[fit_data_bundle_index]);
				booky2.createBooky(filepath.str());
			}
		}

		// single reco plots
		NeatPlotting::PlotStyle single_plot_style;
		single_plot_style.x_axis_style.axis_text_style.text_size = 0.06;
		single_plot_style.y_axis_style.axis_text_style.text_size = 0.06;
		single_plot_style.z_axis_style.axis_text_style.text_size = 0.06;
		single_plot_style.x_axis_style.axis_title_text_offset = 0.95;
		single_plot_style.y_axis_style.axis_title_text_offset = 0.95;
		single_plot_style.z_axis_style.axis_title_text_offset = 0.95;

		NeatPlotting::PlotStyle diff_plot_style(single_plot_style);
		diff_plot_style.palette_color_style = 1;

		single_plot_style.z_axis_style.log_scale = true;

		// ---------- reco -- full phi stuff
		LumiFit::LmdDimensionOptions lmd_dim_opt;
		if (full_phi_vec[fit_data_bundle_index].getElasticDataBundles()[0].getPrimaryDimension().dimension_options.dimension_type
				== LumiFit::THETA_X)
			lmd_dim_opt.dimension_type = LumiFit::THETA_X;

		LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
		full_phi_reco_data_vec = lmd_data_facade.filterData(
				full_phi_vec[fit_data_bundle_index].getElasticDataBundles(), filter);

		for (unsigned int reco_data_obj_index = 0;
				reco_data_obj_index < full_phi_reco_data_vec.size();
				++reco_data_obj_index) {

			// get rid of other cuts (i.e. on primary particles...)
			if (full_phi_reco_data_vec[reco_data_obj_index].getSelectorSet().size()
					> 0)
				continue;

			const map<PndLmdFitOptions, ModelFitResult>& fit_results =
					full_phi_reco_data_vec[reco_data_obj_index].getFitResults();

			map<PndLmdFitOptions, ModelFitResult>::const_iterator fit_result_iter;
			for (fit_result_iter = fit_results.begin();
					fit_result_iter != fit_results.end(); ++fit_result_iter) {
				c.Clear();
				if (make_1d_plots
						&& fit_result_iter->first.getModelOptionsPropertyTree().get<
								unsigned int>("fit_dimension") == 1) {
					NeatPlotting::PlotBundle reco_plot_bundle =
							lmd_plotter.makeGraphBundle(
									full_phi_reco_data_vec[reco_data_obj_index],
									fit_result_iter->first, true, true, false);
					reco_plot_bundle.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_reco.pdf";
					c.SaveAs(filepath.str().c_str());

					unsigned int used_acc_index =
							full_phi_reco_data_vec[reco_data_obj_index].getUsedAcceptanceIndices()[0];

					NeatPlotting::PlotBundle acc_bundle_1d =
							lmd_plotter.makeAcceptanceBundle1D(
									full_phi_vec[fit_data_bundle_index].getUsedAcceptancesPool()[used_acc_index]);
					acc_bundle_1d.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/acceptance1d.pdf";
					c.SaveAs(filepath.str().c_str());
				}

				// save additional root file for tdr...
				/*TGraph* grmodel = reco_plot_bundle.getGraphs()[0].data_object;
				 TH1D* histdata =
				 (TH1D*) reco_plot_bundle.getHistograms()[0].data_object;
				 filepath.str("");
				 filepath << filepath_base.str() << "/fit_result_reco.root";
				 TDirectory *curdir = gDirectory;
				 TFile *ftemp = new TFile(filepath.str().c_str(), "RECREATE");
				 grmodel->Write("model");
				 histdata->Write("data");
				 TNamed label("reldiff_label",
				 reco_plot_bundle.plot_decoration.labels[reco_plot_bundle.plot_decoration.labels.size()
				 - 1].getTitle());
				 label.Write();
				 ftemp->Write();
				 ftemp->Close();
				 if (curdir)
				 curdir->cd();*/

				c.Clear();
				if (make_2d_plots
						&& fit_result_iter->first.getModelOptionsPropertyTree().get<
								unsigned int>("fit_dimension") == 2) {
					NeatPlotting::PlotBundle reco_plot_bundle =
							lmd_plotter.makeGraphBundle(
									full_phi_reco_data_vec[reco_data_obj_index],
									fit_result_iter->first, true, false, false);
					reco_plot_bundle.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_reco_data_2d.pdf";
					c.SaveAs(filepath.str().c_str());

					reco_plot_bundle = lmd_plotter.makeGraphBundle(
							full_phi_reco_data_vec[reco_data_obj_index],
							fit_result_iter->first, false, true, false);
					reco_plot_bundle.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_reco_model_2d.pdf";
					c.SaveAs(filepath.str().c_str());

					reco_plot_bundle = lmd_plotter.makeGraphBundle(
							full_phi_reco_data_vec[reco_data_obj_index],
							fit_result_iter->first, true, true, false);
					reco_plot_bundle.drawOnCurrentPad(diff_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_reco_diff_2d.pdf";
					c.SaveAs(filepath.str().c_str());

					unsigned int used_acc_index =
							full_phi_reco_data_vec[reco_data_obj_index].getUsedAcceptanceIndices()[0];

					NeatPlotting::PlotBundle acc_bundle_2d =
							lmd_plotter.makeAcceptanceBundle2D(
									full_phi_vec[fit_data_bundle_index].getUsedAcceptancesPool()[used_acc_index]);
					//acc_bundle_2d.plot_axis.x_axis_range.active = true;
					//acc_bundle_2d.plot_axis.x_axis_range.low = 0.002;
					//acc_bundle_2d.plot_axis.x_axis_range.high = 0.01;
					single_plot_style.z_axis_style.log_scale = false;
					acc_bundle_2d.drawOnCurrentPad(single_plot_style);
					single_plot_style.z_axis_style.log_scale = true;
					filepath.str("");
					filepath << filepath_base.str() << "/acceptance2d.png";
					c.SaveAs(filepath.str().c_str());
				}
			}
		}

		lmd_dim_opt.track_type = LumiFit::MC;

		LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter_mc(
				lmd_dim_opt);
		std::vector<PndLmdElasticDataBundle> full_phi_mc_data_vec =
				lmd_data_facade.filterData(
						full_phi_vec[fit_data_bundle_index].getElasticDataBundles(),
						filter_mc);

		if (full_phi_mc_data_vec.size() == 1) {
			const map<PndLmdFitOptions, ModelFitResult>& fit_results =
					full_phi_mc_data_vec[0].getFitResults();
			if (fit_results.size() > 0) {

				if (make_2d_plots) {
					c.Clear();
					NeatPlotting::PlotBundle mc_plot_bundle = lmd_plotter.makeGraphBundle(
							full_phi_mc_data_vec[0], fit_results.begin()->first, true, false,
							false);
					mc_plot_bundle.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_mc_data_2d.pdf";
					c.SaveAs(filepath.str().c_str());
				}
			}
		}

		std::cout << "mc accepted case..." << std::endl;
		lmd_dim_opt.track_type = LumiFit::MC_ACC;

		LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter_mc_acc(
				lmd_dim_opt);
		std::vector<PndLmdElasticDataBundle> full_phi_mc_acc_data_vec =
				lmd_data_facade.filterData(
						full_phi_vec[fit_data_bundle_index].getElasticDataBundles(),
						filter_mc_acc);

		if (full_phi_mc_acc_data_vec.size() == 1) {
			const map<PndLmdFitOptions, ModelFitResult>& fit_results =
					full_phi_mc_acc_data_vec[0].getFitResults();

			if (fit_results.size() > 0) {

				if (make_2d_plots) {
					c.Clear();
					NeatPlotting::PlotBundle mc_acc_plot_bundle =
							lmd_plotter.makeGraphBundle(full_phi_mc_acc_data_vec[0],
									fit_results.begin()->first, true, false, false);
					mc_acc_plot_bundle.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_mc_acc_data_2d.pdf";
					c.SaveAs(filepath.str().c_str());

					mc_acc_plot_bundle = lmd_plotter.makeGraphBundle(
							full_phi_mc_acc_data_vec[0], fit_results.begin()->first, false,
							true, false);
					mc_acc_plot_bundle.drawOnCurrentPad(single_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_mc_acc_model_2d.pdf";
					c.SaveAs(filepath.str().c_str());

					mc_acc_plot_bundle = lmd_plotter.makeGraphBundle(
							full_phi_mc_acc_data_vec[0], fit_results.begin()->first, true,
							true, false);
					mc_acc_plot_bundle.drawOnCurrentPad(diff_plot_style);
					filepath.str("");
					filepath << filepath_base.str() << "/fit_result_mc_acc_diff_2d.pdf";
					c.SaveAs(filepath.str().c_str());
				}
			}
		}

	}
// ================================ END PLOTTING ================================ //
}

void displayInfo() {
// display info
	std::cout << "Required arguments are: " << std::endl;
	std::cout << "list of directories to be scanned for vertex data" << std::endl;
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
		plotLumiFitResults(paths, filter_string, output_directory_path,
				filename_suffix);
	}

	return 0;
}
