#include "PndLmdRuntimeConfiguration.h"
#include "fit/PndLmdFitFacade.h"
#include "data/PndLmdDataFacade.h"
#include "data/PndLmdAngularData.h"

#include <iostream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

#include "TFile.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void runLmdFit(string input_file_dir, string config_file_url,
		string acceptance_file_dir, string reference_acceptance_file_dir,
		unsigned int nthreads) {

	boost::chrono::thread_clock::time_point start =
			boost::chrono::thread_clock::now();

	PndLmdRuntimeConfiguration& lmd_runtime_config =
			PndLmdRuntimeConfiguration::Instance();
	lmd_runtime_config.setNumberOfThreads(nthreads);
	lmd_runtime_config.readFitConfigFile(config_file_url);

	lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
	lmd_runtime_config.setAcceptanceResolutionInputDirectory(acceptance_file_dir);
	lmd_runtime_config.setReferenceAcceptanceResolutionInputDirectory(
			reference_acceptance_file_dir);

	// ============================== read data ============================== //

	// get lmd data and objects from files
	PndLmdDataFacade lmd_data_facade;

	vector<PndLmdAngularData> my_lmd_data_vec = lmd_data_facade.getElasticData();

	// filter out specific data
	LumiFit::LmdDimensionOptions lmd_dim_opt;
	lmd_dim_opt.dimension_type = LumiFit::THETA_X;
	lmd_dim_opt.track_type = LumiFit::MC_ACC;
	LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
	my_lmd_data_vec = lmd_data_facade.filterData<PndLmdAngularData>(
			my_lmd_data_vec, filter);

	vector<PndLmdAcceptance> my_lmd_acc_vec = lmd_data_facade.getAcceptanceData();
	vector<PndLmdHistogramData> all_lmd_res = lmd_data_facade.getResolutionData();

	// ------------------------------------------------------------------------

	// start fitting
	PndLmdFitFacade lmd_fit_facade;
	// add acceptance data to pools
	// the corresponding acceptances to the data will automatically be taken
	// if not found then this fit is skipped
	lmd_fit_facade.addAcceptencesToPool(my_lmd_acc_vec);
	lmd_fit_facade.addResolutionsToPool(all_lmd_res);

	// do actual fits
	PndLmdFitDataBundle fit_result(
			lmd_fit_facade.doLuminosityFits(my_lmd_data_vec));

	// open file via runtime config and save results to file
	// create output file
	std::stringstream hs;
	hs << lmd_runtime_config.getElasticDataInputDirectory().string() << "/"
			<< lmd_runtime_config.getFittedElasticDataName();

	fit_result.saveDataBundleToRootFile(hs.str());

	boost::chrono::thread_clock::time_point stop =
			boost::chrono::thread_clock::now();
	std::cout << "duration: "
			<< boost::chrono::duration_cast<boost::chrono::milliseconds>(stop - start).count()
					/ 60000.0 << " min\n";
}

void displayInfo() {
	// display info
	cout << "Required arguments are: " << endl;
	cout << "-d [path to data]" << endl;
	cout << "-c [path to config file] " << endl;
	cout << "Optional arguments are: " << endl;
	cout << "-m [number of threads]" << endl;
	cout << "-a [path to box gen data] (acceptance)" << endl;
	cout << "-r [path to reference box gen data] (acceptance)" << endl;
}

int main(int argc, char* argv[]) {
	string data_path;
	string acc_path("");
	string config_path("");
	string ref_acc_path("");
	unsigned int nthreads(1);
	bool is_data_set(false), is_config_set(false), is_acc_set(false),
			is_nthreads_set(false);

	int c;

	while ((c = getopt(argc, argv, "hc:a:m:r:d:")) != -1) {
		switch (c) {
			case 'a':
				acc_path = optarg;
				is_acc_set = true;
				break;
			case 'c':
				config_path = optarg;
				is_config_set = true;
				break;
			case 'r':
				ref_acc_path = optarg;
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
				if (optopt == 'm' || optopt == 'd' || optopt == 'a' || optopt == 'c'
						|| optopt == 'r')
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
		runLmdFit(data_path, config_path, acc_path, ref_acc_path, nthreads);
	else
		displayInfo();
	return 0;
}

