#include "PndLmdLumiHelper.h"
#include "ROOTPlotHelper.hpp"
#include "data/PndLmdDataFacade.h"
#include "data/PndLmdHistogramData.h"
#include "fit/ModelFitFacade.h"
#include "fit/PndLmdLumiFitResult.h"
#include "fit/data/DataStructs.h"
#include "fit/data/ROOT/ROOTDataHelper.h"
#include "fit/estimatorImpl/Chi2Estimator.h"
#include "fit/estimatorImpl/LogLikelihoodEstimator.h"
#include "fit/estimatorImpl/UnbinnedLogLikelihoodEstimator.h"
#include "fit/minimizerImpl/ROOT/ROOTMinimizer.h"
#include "model/AsymmetricGaussianModel1D.h"
#include "operators1d/AdditionModel1D.h"
#include "visualization/ModelVisualizationProperties1D.h"
#include "visualization/ROOT/ROOTPlotter.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TSpectrum.h"

void fitTestResolution(std::string file_path, int slice_index) {
  // get data
  std::stringstream infile_path;
  infile_path << file_path << "/lmd_res_data.root";

  TFile *infile = new TFile(infile_path.str().c_str(), "READ");

  PndLmdDataFacade lmd_data_facade;
  vector<PndLmdHistogramData> all_lmd_res =
      lmd_data_facade.getDataFromFile<PndLmdHistogramData>(infile);

  if (slice_index < 0)
    slice_index = 9;

  if (all_lmd_res.size() > slice_index) {

    std::cout << "using slice: " << slice_index << std::endl;

    TH1D *hist(all_lmd_res[slice_index].get1DHistogram());

    // create model

    shared_ptr<AsymmetricGaussianModel1D> narrow_gaus(
        new AsymmetricGaussianModel1D("narrow_gaus"));
    shared_ptr<AsymmetricGaussianModel1D> wide_gaus(
        new AsymmetricGaussianModel1D("wide_gaus"));

    shared_ptr<Model1D> double_asymm_gaus(
        new AdditionModel1D("double_asymm_gaus", narrow_gaus, wide_gaus));

    // set start parameters

    TSpectrum peak_finder(2); // max 2 peaks
    int num_found_peaks = peak_finder.Search(hist, 3, "nobackground");

    double sigma_n = 0.2 * hist->GetRMS();
    double mean_n = peak_finder.GetPositionX()[0];
    double amplitude_n =
        0.7 * peak_finder.GetPositionY()[0] / sqrt(2.0 * 3.14 * sigma_n);

    double sigma_w = hist->GetRMS();
    double mean_w = peak_finder.GetPositionX()[0];
    double amplitude_w =
        0.3 * peak_finder.GetPositionY()[0] / sqrt(2.0 * 3.14 * sigma_w);

    if (num_found_peaks == 2) {
      int index_narrow(0);
      int index_wide(1);
      if (peak_finder.GetPositionY()[0] < peak_finder.GetPositionY()[1]) {
        index_narrow = 1;
        index_wide = 0;
      }
      sigma_n = 0.2 * hist->GetRMS();
      mean_n = peak_finder.GetPositionX()[index_narrow];
      amplitude_n =
          peak_finder.GetPositionY()[index_narrow] / sqrt(2.0 * 3.14 * sigma_n);

      sigma_w = hist->GetRMS();
      mean_w = peak_finder.GetPositionX()[index_wide];
      amplitude_w =
          peak_finder.GetPositionY()[index_wide] / sqrt(2.0 * 3.14 * sigma_w);
    }

    double_asymm_gaus->getModelParameterSet().freeAllModelParameters();

    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(
            std::make_pair("narrow_gaus", "asymm_gauss_sigma_left"))
        ->setValue(sigma_n);
    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(
            std::make_pair("narrow_gaus", "asymm_gauss_sigma_right"))
        ->setValue(sigma_n);
    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(std::make_pair("narrow_gaus", "asymm_gauss_mean"))
        ->setValue(mean_n);
    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(
            std::make_pair("narrow_gaus", "asymm_gauss_amplitude"))
        ->setValue(amplitude_n);

    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(
            std::make_pair("wide_gaus", "asymm_gauss_sigma_left"))
        ->setValue(sigma_w);
    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(
            std::make_pair("wide_gaus", "asymm_gauss_sigma_right"))
        ->setValue(sigma_w);
    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(std::make_pair("wide_gaus", "asymm_gauss_mean"))
        ->setValue(mean_w);
    double_asymm_gaus->getModelParameterSet()
        .getModelParameter(std::make_pair("wide_gaus", "asymm_gauss_amplitude"))
        ->setValue(amplitude_w);

    // model->getModelParameterSet().printInfo();

    // lets do the fitting

    ModelFitFacade fit_facade;

    shared_ptr<LogLikelihoodEstimator> loglikelihood_est(
        new LogLikelihoodEstimator());
    fit_facade.setEstimator(loglikelihood_est);
    shared_ptr<Data> data(new Data(1));
    ROOTDataHelper data_helper;
    data_helper.fillBinnedData(data, hist);
    fit_facade.setData(data);

    fit_facade.setModel(double_asymm_gaus);

    // create minimizer instance with control parameter
    shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());

    fit_facade.setMinimizer(minuit_minimizer);

    DataStructs::DimensionRange fit_range_th(hist->GetXaxis()->GetXmin(),
                                             hist->GetXaxis()->GetXmax());

    EstimatorOptions est_opt;
    est_opt.setWithIntegralScaling(true);
    est_opt.setFitRangeX(fit_range_th);
    // est_opt.setFitRangeY(plot_range_phi);
    fit_facade.setEstimatorOptions(est_opt);

    ModelFitResult fit_result = fit_facade.Fit();

    ROOTPlotter root_plotter;

    ModelVisualizationProperties1D vis_prop_th;
    vis_prop_th.setBinningFactor(hist->GetXaxis()->GetBinWidth(1));
    vis_prop_th.setPlotRange(fit_range_th);

    TGraphAsymmErrors *graph =
        root_plotter.createGraphFromModel1D(double_asymm_gaus, vis_prop_th);

    TCanvas c;

    hist->Draw("E1");
    hist->GetXaxis()->SetTitle("#theta / rad");
    hist->GetYaxis()->SetTitle("#entries");
    std::stringstream titletext;
    // titletext << "reldiff = " << lumi_reldiff.first << " #pm "
    //		<< lumi_reldiff.second << " %";
    // hist->SetTitle(titletext.str().c_str());
    graph->SetLineColor(2);
    graph->Draw("CSAME");
    c.SaveAs("resolution_test.pdf");
  }
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "-p [path to resolution slice data]" << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-s [resolution slice index]" << std::endl;
}

int main(int argc, char *argv[]) {
  bool is_data_path_set = false;
  int index = -1;
  std::string data_path;
  int c;

  while ((c = getopt(argc, argv, "hp:s:")) != -1) {
    switch (c) {
    case 's':
      index = atoi(optarg);
      break;
    case 'p':
      data_path = optarg;
      is_data_path_set = true;
      break;
    case '?':
      if (optopt == 'p' || optopt == 's')
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

  if (is_data_path_set) {
    fitTestResolution(data_path, index);
  } else
    displayInfo();

  return 0;
}
