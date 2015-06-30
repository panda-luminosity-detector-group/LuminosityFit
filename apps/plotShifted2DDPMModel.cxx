#include "PndLmdPlotter.h"
#include "fit/PndLmdFitFacade.h"

#include "PndLmdRuntimeConfiguration.h"

#include <iostream>
#include <string>

#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono/thread_clock.hpp>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void plot2DModel(string input_file_dir, string config_file_url,
		string acceptance_file_dir, unsigned int nthreads) {

	boost::chrono::thread_clock::time_point start =
			boost::chrono::thread_clock::now();

	PndLmdRuntimeConfiguration& lmd_runtime_config = PndLmdRuntimeConfiguration::Instance();
	lmd_runtime_config.setNumberOfThreads(nthreads);

	lmd_runtime_config.setElasticDataInputDirectory(input_file_dir);
	lmd_runtime_config.setAcceptanceResolutionInputDirectory(acceptance_file_dir);

	LumiFit::LmdDimensionOptions data_dim_opt;
	data_dim_opt.track_type = LumiFit::RECO;

	PndLmdFitFacade lmd_fit_facade;
	shared_ptr<Model> model = lmd_fit_facade.generateModel(data_dim_opt);

	const double theta_min = -0.01;
	const double theta_max = 0.01;

	DataStructs::DimensionRange plot_range_thx(theta_min, theta_max);
	DataStructs::DimensionRange plot_range_thy(theta_min, theta_max);

	// create 2d model and initialize tilt parameters
  double tilt_x(0.0);
  double tilt_y(0.0);

	model->getModelParameterSet().freeModelParameter("tilt_x");
	model->getModelParameterSet().freeModelParameter("tilt_y");
	model->getModelParameterSet().setModelParameterValue("tilt_x", tilt_x);
	model->getModelParameterSet().setModelParameterValue("tilt_y", tilt_y);

	model->getModelParameterSet().printInfo();

	// integral - just for testing purpose
	/*std::vector<DataStructs::DimensionRange> int_range;
	DataStructs::DimensionRange dr_th(0.001, 0.01);
	int_range.push_back(dr_th);
	DataStructs::DimensionRange dr_phi(-TMath::Pi(), TMath::Pi());
	int_range.push_back(dr_phi);
	double integral_value = model->Integral(int_range, 1e-3);
	std::cout << "integral: " << integral_value << std::endl;*/

	//plot result
	ROOTPlotter root_plotter;

	ModelVisualizationProperties1D vis_prop_th;
	vis_prop_th.setPlotRange(plot_range_thx);
	vis_prop_th.setEvaluations(200);

	ModelVisualizationProperties1D vis_prop_phi;
	vis_prop_phi.setPlotRange(plot_range_thy);
	vis_prop_phi.setEvaluations(200);

	std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D> vis_prop_pair =
			std::make_pair(vis_prop_th, vis_prop_phi);

	TH2D* model_hist = root_plotter.createHistogramFromModel2D(model,
			vis_prop_pair);

	TCanvas c;

	model_hist->Draw("colz");
	model_hist->GetXaxis()->SetTitle("#theta / rad");
	model_hist->GetYaxis()->SetTitle("#phi / rad");
	std::stringstream titletext;
	titletext << "tilt_{x}=" << tilt_x << " rad, tilt_{y}=" << tilt_y << " rad";
	model_hist->SetTitle(titletext.str().c_str());

	//c.SetLogz(1);

	std::stringstream strstream;
	strstream.precision(3);

	strstream << "DPMModel2D_tiltxy-" << tilt_x << "-" << tilt_y << "_plab-"
			<< lmd_runtime_config.getMomentum() << ".pdf";
	c.SaveAs(strstream.str().c_str());
	strstream.str("");
	strstream << "DPMModel2D_tiltxy-" << tilt_x << "-" << tilt_y << "_plab-"
			<< lmd_runtime_config.getMomentum() << ".png";
	c.SaveAs(strstream.str().c_str());

	boost::chrono::thread_clock::time_point stop =
			boost::chrono::thread_clock::now();
	std::cout << "duration: "
			<< boost::chrono::duration_cast<boost::chrono::milliseconds>(stop - start).count()
			<< " ms\n";
}

void displayInfo() {
	// display info
	cout << "Required arguments are: " << endl;
	cout << "-d [path to data]" << endl;
	cout << "-c [path to config file] " << endl;
	cout << "Optional arguments are: " << endl;
	cout << "-m [number of threads]" << endl;
	cout << "-a [path to box gen data] (acceptance)" << endl;
}

int main(int argc, char* argv[]) {
	string data_path;
	string acc_path("");
	string config_path("");
	unsigned int nthreads(1);
	bool is_data_set(false), is_config_set(false), is_acc_set(false),
			is_nthreads_set(false);

	int c;

	while ((c = getopt(argc, argv, "hc:a:m:d:")) != -1) {
		switch (c) {
			case 'a':
				acc_path = optarg;
				is_acc_set = true;
				break;
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
				if (optopt == 'm' || optopt == 'd' || optopt == 'a' || optopt == 'c')
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
		plot2DModel(data_path, config_path, acc_path, nthreads);
	else
		displayInfo();
	return 0;
}

