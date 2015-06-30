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

#include "data/PndLmdAngularData.h"
#include "data/PndLmdDataFacade.h"

#include <vector>
#include <map>
#include <iostream>
#include <iterator>

#include "TString.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TGraphErrors.h"

#include "boost/regex.hpp"

void plotMultipleLumiFitResults(std::vector<std::string> paths,
		TString filename_suffix) {
	std::cout << "Generating lumi plots for fit results....\n";

	// A small helper class that helps to construct lmd data objects
	PndLmdDataFacade lmd_data_facade;

	// ================================ BEGIN CONFIG ================================ //
	// PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
	// you can fine tune it and overwrite the default values
	//gStyle->SetPadTopMargin(0.06);
	//gStyle->SetPadBottomMargin(0.12);
	gStyle->SetPadLeftMargin(0.15);
	//gStyle->SetPadRightMargin(0.1);

	// ================================= END CONFIG ================================= //

	std::vector<double> x_values;
	std::vector<double> y_values;
	std::vector<double> y_errors;

	for (unsigned int i = 0; i < paths.size(); i++) {
		// ------ get files -------------------------------------------------------
		TFile *fdata = new TFile((paths[i] + "/lmd_fitted_data.root").c_str(),
				"READ");

		const boost::regex my_filename_filter("phi_slice_(-?\\d*)",
				boost::regex::extended | boost::regex::icase);

		boost::smatch fwhat;

		double x_val = 0.0;
		// Skip if no match
		if (!boost::regex_search(paths[i], fwhat, my_filename_filter))
			continue;
		else {
			std::string blub = fwhat[1].str();
			std::cout << blub << std::endl;
			x_val = atof(blub.c_str());
		}

		// read in data from a root file which will return a map of pointers to PndLmdAngularData objects
		std::vector<PndLmdAngularData> data_vec = lmd_data_facade.getDataFromFile<
				PndLmdAngularData>(fdata);

		PndLmdAngularData reco;
		for (unsigned int j = 0; j < data_vec.size(); j++) {
			if (data_vec[j].getPrimaryDimension().dimension_options.track_type
					== LumiFit::RECO) {
				reco = data_vec[j];
				break;
			}
		}

		const map<PndLmdFitOptions, PndLmdLumiFitResult*> fit_results =
				reco.getFitResults();

		if (fit_results.begin()->second->getModelFitResult().getFitStatus() == 0) {
			double lumi = fit_results.begin()->second->getLuminosity();
			double lumi_err = fit_results.begin()->second->getLuminosityError();
			double lumi_ref = reco.getReferenceLuminosity();

			std::cout<<(lumi - lumi_ref) / lumi_ref << " +- " << lumi_err / lumi_ref << std::endl;
			x_values.push_back(x_val);
			y_values.push_back(100.0*(lumi - lumi_ref) / lumi_ref);
			y_errors.push_back(100.0*lumi_err / lumi_ref);
		}
	}

	TString s("lumifit_results_reco_phi_sliced");
	s += filename_suffix + ".pdf";

	TCanvas c;
	TGraphErrors *graph = new TGraphErrors(x_values.size(), &x_values[0],
			&y_values[0], 0, &y_errors[0]);

	graph->SetTitle("");
	graph->GetXaxis()->SetTitle("#phi [1/#frac{#pi}{5}]");
	graph->GetYaxis()->SetTitle("#Delta L/L [%]");
	graph->SetMarkerStyle(10);
	graph->SetMarkerSize(0.06);
	graph->Draw("AP");
	c.SaveAs(s);

	// ================================ END PLOTTING ================================ //
}

void displayInfo() {
	// display info
	std::cout << "Required arguments are: " << std::endl;
	std::cout << "list of directories to be scanned for lmd angular data"
			<< std::endl;
	std::cout << "Optional arguments are: " << std::endl;
	std::cout << "-f [output filename suffix]" << std::endl;
}

int main(int argc, char* argv[]) {
	bool is_filename_suffix_set = false;
	std::string filename_suffix("");

	int c;

	while ((c = getopt(argc, argv, "hf:")) != -1) {
		switch (c) {
			case 'f':
				filename_suffix = optarg;
				is_filename_suffix_set = true;
				break;
			case '?':
				if (optopt == 'f')
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

	if (argc > 2) {
		std::vector<std::string> paths;
		for (unsigned int i = argoffset; i < argc; i++) {
			paths.push_back(std::string(argv[i]));
		}
		plotMultipleLumiFitResults(paths, filename_suffix);
	}

	return 0;
}
