#include "data/PndLmdDataFacade.h"
#include "fit/PndLmdFitFacade.h"
#include "LumiFitStructs.h"

#include <iostream>
#include <sstream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

#include "TFile.h"

using std::string;
using std::cerr;
using std::endl;
using std::cout;

void determineBeamOffset(string input_file_dir, string config_file_url,
		unsigned int nthreads) {

	boost::chrono::thread_clock::time_point start =
			boost::chrono::thread_clock::now();

	PndLmdRuntimeConfiguration& lmd_runtime_config =
			PndLmdRuntimeConfiguration::Instance();
	lmd_runtime_config.setNumberOfThreads(nthreads);
	lmd_runtime_config.readFitConfigFile(config_file_url);

	// facade for opening lmd files
	PndLmdDataFacade lmd_data_facade;

	// then open the vertex distribution root file and get the lmd data objects
	std::stringstream hs;
	hs << input_file_dir << "/lmd_vertex_data.root";

	TFile f(hs.str().c_str(), "READ");
	std::vector<PndLmdHistogramData> data_vec =
			lmd_data_facade.getDataFromFile<PndLmdHistogramData>(f);

	PndLmdFitFacade lmd_fit_facade;

	// do fits
	lmd_fit_facade.fitVertexData(data_vec);

	// save data
	std::cout << "Saving data...." << std::endl;
	// output file
	hs.str("");
	hs << input_file_dir << "/lmd_fitted_vertex_data.root";
	TFile *ffitteddata = new TFile(hs.str().c_str(), "RECREATE");

	for (std::vector<PndLmdHistogramData>::iterator lmd_data_iter =
			data_vec.begin(); lmd_data_iter != data_vec.end();
			lmd_data_iter++) {
		std::cout<<lmd_data_iter->getName()<<": "<<lmd_data_iter->getFitResults().size()<<std::endl;
		if (lmd_data_iter->getFitResults().size() > 0)
      lmd_data_iter->saveToRootFile();
	}

	ffitteddata->Close();
}

void displayInfo() {
	// display info
	cout << "Required arguments are: " << endl;
	cout << "-d [path to data]" << endl;
	cout << "-c [path to config file] " << endl;
	cout << "Optional arguments are: " << endl;
	cout << "-m [number of threads]" << endl;
}

int main(int argc, char* argv[]) {
	string data_path;
	string config_path("");
	unsigned int nthreads(1);
	bool is_data_set(false), is_config_set(false), is_nthreads_set(false);

	int c;

	while ((c = getopt(argc, argv, "hc:m:d:")) != -1) {
		switch (c) {
			case 'c':
				config_path = optarg;
				is_config_set = true;
				break;
			case 'd':
				data_path = optarg;
				is_data_set = true;
				break;
			case 'm':
				nthreads = atoi(optarg);
				is_nthreads_set = true;
				break;
			case '?':
				if (optopt == 'm' || optopt == 'd' || optopt == 'c')
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

	if (is_data_set && is_config_set)
		determineBeamOffset(data_path, config_path, nthreads);
	else
		displayInfo();
	return 0;
}
