#include "data/PndLmdDataFacade.h"
#include "PndLmdPlotter.h"
#include "fit/PndLmdLumiFitResult.h"
#include "data/PndLmdAcceptance.h"
#include "model/PndLmdROOTDataModel1D.h"

#include <iostream>               // for std::cout
#include <utility>
#include <vector>
#include <sstream>

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TAxis.h"

void compareAcceptances(std::vector<std::string> &acceptance_files) {
	std::cout << "Generating acceptance comparison plots ....\n";

	// create an instance of PndLmdPlotter the plotting helper class
	LumiFit::PndLmdPlotter lmd_plotter;

	PndLmdDataFacade lmd_data_facade;

	// create set of acceptances
	std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
			std::vector<PndLmdAcceptance> > acc_map;
	for (unsigned int i = 0; i < acceptance_files.size(); i++) {
		TFile facc(acceptance_files[i].c_str(), "READ");
		std::vector<PndLmdAcceptance> accs = lmd_data_facade.getDataFromFile<
				PndLmdAcceptance>(facc);
		for (unsigned int j = 0; j < accs.size(); j++) {
			acc_map[std::make_pair(accs[j].getPrimaryDimension(),
					accs[j].getSecondaryDimension())].push_back(accs[j]);
		}
	}

	std::vector<std::pair<PndLmdAcceptance, PndLmdAcceptance> > matched_acc_pairs;

	std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
			std::vector<PndLmdAcceptance> >::iterator it;
	for (it = acc_map.begin(); it != acc_map.end(); it++) {
		std::vector<PndLmdAcceptance> matching_acc_vec = it->second;

		if (matching_acc_vec.size() > 1) {
			for (unsigned int i = 1; i < matching_acc_vec.size(); i++) {
				// just take the first object as the reference
				matched_acc_pairs.push_back(
						std::make_pair(matching_acc_vec[0], matching_acc_vec[i]));
			}
		}
	}

	std::cout << "paired " << matched_acc_pairs.size() << " acceptances!"
			<< std::endl;

	lmd_plotter.createAcceptanceComparisonBooky(matched_acc_pairs);
}

int main(int argc, char* argv[]) {
	if (argc >= 2) {
		std::vector<std::string> acceptance_file_urls;
		for (unsigned int i = 1; i < argc; i = i++)
			acceptance_file_urls.push_back(std::string(argv[i]));
		compareAcceptances(acceptance_file_urls);
		return 0;
	}
	return 1;
}
