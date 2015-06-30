#include "data/PndLmdDataFacade.h"
#include "PndLmdPlotter.h"
#include "fit/PndLmdLumiFitResult.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"
#include "LumiFitStructs.h"
#include "PndLmdComparisonStructs.h"

#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLine.h"
#include "TMultiGraph.h"
#include "TLegend.h"

#include <iostream>               // for std::cout
#include <map>
#include <vector>
#include <sstream>

using std::cout;
using std::endl;
using std::cerr;
using std::map;
using std::vector;

/*void saveToRootfile(std::map<PndLmdLumiFitOptions, histBundle> &comb_map,
 TFile *file) {

 if (comb_map.size() == 0)
 return;

 std::stringstream topdir_name;
 topdir_name << "fit_opt-" << comb_map.begin()->first.getFitModelOptions();

 file->mkdir(topdir_name.str().c_str());
 gDirectory->cd(topdir_name.str().c_str());
 for (std::map<PndLmdLumiFitOptions, histBundle>::iterator it =
 comb_map.begin(); it != comb_map.end(); it++) {
 double mean_error = it->second.mean_rel_diff_lumi_error
 / it->second.num_entries;
 it->second.hist->Draw("E1");
 std::stringstream s;
 s.precision(3);
 s << "lumifit error on rel diff: " << mean_error;
 TLatex *label = new TLatex(xmin * 0.9, 40.0, s.str().c_str());
 label->Draw();
 s.str("");
 s << "#Theta fit range: "
 << it->first.getEstimatorOptions().getFitRangeX().range_low << " - "
 << it->first.getEstimatorOptions().getFitRangeX().range_high;
 TLatex *label2 = new TLatex(xmin * 0.9, 36.0, s.str().c_str());
 label2->Draw();

 file->cd();
 gDirectory->cd(topdir_name.str().c_str());
 s.str("");
 s << it->first.getEstimatorOptions().getFitRangeX().range_low << " - "
 << it->first.getEstimatorOptions().getFitRangeX().range_high;
 gDirectory->mkdir(s.str().c_str());
 gDirectory->cd(s.str().c_str());
 it->second.hist->Write("reldiff_dist");
 label->Write("error_label");
 label2->Write("fit_range_label");
 }
 }*/

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

void determineLumiFitSystematics(std::string pathname,
		std::string dir_name_filter, std::string dependency_suffix) {
	std::cout << "Generating lumi comparison plots for fit results....\n";

	// create an instance of PndLmdDataFacade for retrieving data
	PndLmdDataFacade lmd_data_facade;

	vector<std::string> paths = lmd_data_facade.findFilesByName(pathname,
			dir_name_filter, "lmd_fitted_data.root");

	std::vector<PndLmdAngularData> mc_data_vec;
	std::vector<PndLmdAngularData> mc_acc_data_vec;
	std::vector<PndLmdAngularData> reco_data_vec;

	for (unsigned int j = 0; j < paths.size(); j++) {
		std::cout << "working on: " << paths[j] << std::endl;

		// ------ get file -------------------------------------------------------
		TFile fdata((paths[j] + "/lmd_fitted_data.root").c_str(),
				"UPDATE");

		// read in data from a root file which will return a map of pairs dimension options to
		// vectors of pointers to PndLmdAngularData objects
		vector<PndLmdAngularData> full_data_vec = lmd_data_facade.getDataFromFile<
				PndLmdAngularData>(fdata);

		// MC data case
		LumiFit::LmdDimensionOptions lmd_dim_opt;
		lmd_dim_opt.track_type = LumiFit::MC;

		LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(
				lmd_dim_opt);
		vector<PndLmdAngularData> data_vec = lmd_data_facade.filterData(
				full_data_vec, filter);

		mc_data_vec.insert(mc_data_vec.end(), data_vec.begin(), data_vec.end());

		// MC ACC data case
		lmd_dim_opt.track_type = LumiFit::MC_ACC;

		filter.setDimensionOptions(lmd_dim_opt);
		data_vec = lmd_data_facade.filterData(full_data_vec, filter);

		mc_acc_data_vec.insert(mc_acc_data_vec.end(), data_vec.begin(),
				data_vec.end());

		// RECO data case
		lmd_dim_opt.track_type = LumiFit::RECO;

		filter.setDimensionOptions(lmd_dim_opt);
		data_vec = lmd_data_facade.filterData(full_data_vec, filter);

		reco_data_vec.insert(reco_data_vec.end(), data_vec.begin(), data_vec.end());
	}

	LumiFit::PndLmdPlotter plotter;

	LumiFit::LmdDimensionOptions dim_op;
	dim_op.track_type = LumiFit::MC;

	NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle mc_graph_bundle =
			plotter.createLowerFitRangeDependencyGraphBundle(mc_data_vec, dim_op);

	dim_op.track_type = LumiFit::MC_ACC;
	NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle mc_acc_graph_bundle =
			plotter.createLowerFitRangeDependencyGraphBundle(mc_acc_data_vec,
					dim_op);

	dim_op.track_type = LumiFit::RECO;
	NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle reco_graph_bundle =
			plotter.createLowerFitRangeDependencyGraphBundle(reco_data_vec, dim_op);

	// save results
	TFile *file = new TFile(
			TString(pathname) + "/lumifit_systematics-"
					+ TString(dependency_suffix.c_str()) + ".root", "RECREATE");

	saveGraphBundlesToFile(mc_graph_bundle, mc_acc_graph_bundle,
			reco_graph_bundle, dependency_suffix);
}

void createDependencyGraphs(std::string pathname,
		std::string dependency_suffix) {
	TFile fdata(pathname.c_str(), "READ");

	// create an instance of PndLmdDataFacade for retrieving data
	PndLmdDataFacade lmd_data_facade;

	LumiFit::PndLmdPlotter plotter;

	// read in data from a root file which will return a map of pairs dimension options to
	// vectors of pointers to PndLmdAngularData objects
	vector<PndLmdAngularData> full_data_vec = lmd_data_facade.getDataFromFile<
			PndLmdAngularData>(fdata);

	// MC data case
	LumiFit::LmdDimensionOptions lmd_dim_opt;
	lmd_dim_opt.track_type = LumiFit::MC;

	LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(
			lmd_dim_opt);
	vector<PndLmdAngularData> data_vec = lmd_data_facade.filterData(full_data_vec,
			filter);
	std::cout << "Got " << data_vec.size() << " data entries!" << std::endl;

	TGraphAsymmErrors* mc_graph = plotter.createLowerFitRangeDependencyGraph(
			data_vec, lmd_dim_opt);

	// MC ACC data case
	lmd_dim_opt.track_type = LumiFit::MC_ACC;

	filter.setDimensionOptions(lmd_dim_opt);
	data_vec = lmd_data_facade.filterData(full_data_vec, filter);

	TGraphAsymmErrors* mc_acc_graph = plotter.createLowerFitRangeDependencyGraph(
			data_vec, lmd_dim_opt);

	// RECO data case
	lmd_dim_opt.track_type = LumiFit::RECO;

	filter.setDimensionOptions(lmd_dim_opt);
	data_vec = lmd_data_facade.filterData(full_data_vec, filter);

	TGraphAsymmErrors* reco_graph = plotter.createLowerFitRangeDependencyGraph(
			data_vec, lmd_dim_opt);

	// save results
	boost::filesystem::path fullpath(pathname);

	TFile *file = new TFile(
			TString(fullpath.remove_filename().c_str()) + "/lumifit_systematics-"
					+ TString(dependency_suffix.c_str()) + ".root", "RECREATE");
	mc_graph->Write("mc");
	mc_acc_graph->Write("mc_acc");
	reco_graph->Write("reco");
	file->Close();
}

void displayInfo() {
// display info
	cout << "Required arguments are: " << endl;
	cout << "-d [path to data]" << endl;
	cout << "-l [suffix label for this data]" << endl;
	cout << "Optional arguments are: " << endl;
	cout << "-b [bunch folder prefix]" << endl;
}

int main(int argc, char* argv[]) {
	bool is_data_set = false, is_suffix_label_set = false,
			is_bunch_folder_prefix_set = false;
	std::string data_path, suffix_label, bunch_folder_prefix;

	int c;

	while ((c = getopt(argc, argv, "hd:l:b:")) != -1) {
		switch (c) {
			case 'd':
				data_path = optarg;
				is_data_set = true;
				break;
			case 'l':
				suffix_label = optarg;
				is_suffix_label_set = true;
				break;
			case 'b':
				bunch_folder_prefix = optarg;
				is_bunch_folder_prefix_set = true;
				break;
			case '?':
				if (optopt == 'd' || optopt == 'l' || optopt == 'b')
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

	if (is_data_set && is_suffix_label_set && is_bunch_folder_prefix_set)
		determineLumiFitSystematics(data_path, bunch_folder_prefix, suffix_label);
	else if (is_data_set && is_suffix_label_set) {
		createDependencyGraphs(data_path, suffix_label);
	} else
		displayInfo();
	return 0;
}

