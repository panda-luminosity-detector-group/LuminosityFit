#include "fit/ModelFitFacade.h"
#include "fit/ModelFitResult.h"
#include "fit/data/Data.h"
#include "fit/data/ROOT/ROOTDataHelper.h"
#include "fit/estimatorImpl/Chi2Estimator.h"
#include "fit/estimatorImpl/LogLikelihoodEstimator.h"
#include "fit/minimizerImpl/ROOT/ROOTMinimizer.h"
#include "models1d/GaussianModel1D.h"

#include <cmath>
#include <map>
#include <sstream>
#include <stdlib.h>

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TMath.h"
#include "TRandom3.h"

double sigma = 1.0;
double mean = 0.0;

TGraphErrors *createGraph(std::map<double, TH1D *> &histograms) {
  TGraphErrors *graph = new TGraphErrors(histograms.size());
  int index = 0;

  TF1 *func = new TF1("myfunc", "gaus(0)", -3, 3);
  func->SetParameters(1000, 0.0, 0.1);
  for (std::map<double, TH1D *>::iterator it = histograms.begin();
       it != histograms.end(); it++) {
    it->second->Fit(func, "R");
    graph->SetPoint(index, it->first, func->GetParameter(1));
    graph->SetPointError(index, 0.0, func->GetParError(1));
    index++;
  }
  return graph;
}

double mygauss(double *x, double *par) {
  return par[0] / (sqrt(2.0 * TMath::Pi()) * sigma) *
         exp(-0.5 * pow((x[0] - mean) / sigma, 2.0));
}

void checkFramework(unsigned int num_events) {
  // gaussian parameters

  double refamp = num_events;

  int samples = 150;

  TRandom3 rand;

  std::map<double, TH1D *> histograms;
  std::map<double, TH1D *> root_histograms;

  TH1D *gaushist;

  for (int sample = 0; sample < samples; sample++) {
    std::cout << "in sample " << sample << std::endl;
    // lets create gaussian random data
    std::cout << "creating histogram " << std::endl;
    gaushist = new TH1D("gaushist", "gaushist", 300, 0.0, 4.0);

    std::cout << "filling histogram with " << num_events
              << " of gaussian distributed random data..." << std::endl;

    for (unsigned int i = 0; i < num_events; i++) {
      gaushist->Fill(rand.Gaus(mean, sigma));
    }
    gaushist->Sumw2();

    // now fit it with a gaussian model
    ModelFitFacade model_fit_facade;

    // create chi2 estimator
    // std::shared_ptr<Chi2Estimator> chi2_est(new Chi2Estimator());
    // model_fit_facade.setEstimator(chi2_est);

    // create loglikelihood estimator
    std::shared_ptr<LogLikelihoodEstimator> loglikelihood_est(
        new LogLikelihoodEstimator());
    model_fit_facade.setEstimator(loglikelihood_est);

    // create a new model
    std::shared_ptr<Model1D> model1d(new GaussianModel1D("gauss"));
    model1d->getModelParameterSet()
        .getModelParameter("gauss_sigma")
        ->setValue(sigma);
    model1d->getModelParameterSet()
        .getModelParameter("gauss_sigma")
        ->setParameterFixed(true);
    model1d->getModelParameterSet()
        .getModelParameter("gauss_mean")
        ->setValue(mean);
    model1d->getModelParameterSet()
        .getModelParameter("gauss_mean")
        ->setParameterFixed(true);
    model1d->getModelParameterSet()
        .getModelParameter("gauss_amplitude")
        ->setValue(refamp);
    model1d->getModelParameterSet()
        .getModelParameter("gauss_amplitude")
        ->setParameterFixed(false);
    // set model
    model_fit_facade.setModel(model1d);

    // create and set data
    ROOTDataHelper data_helper;
    std::shared_ptr<Data> data(new Data(1));
    data_helper.fillBinnedData(data, gaushist);
    model_fit_facade.setData(data);

    double fit_range_low_begin = 0.0;
    double fit_range_low_end = 0.8;
    double fit_range_high = 3.0;

    unsigned int steps = 20;

    for (unsigned int step = 0; step < steps; step++) {
      std::cout << "in step " << step << std::endl;
      DataStructs::DimensionRange fit_range;
      fit_range.range_low =
          fit_range_low_begin +
          (fit_range_low_end - fit_range_low_begin) / steps * step;
      fit_range.range_high = fit_range_high;

      EstimatorOptions est_opt;
      est_opt.setFitRangeX(fit_range);
      est_opt.setWithIntegralScaling(false);

      model_fit_facade.setEstimatorOptions(est_opt);

      // create minimizer instance with control parameter
      std::shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());

      model_fit_facade.setMinimizer(minuit_minimizer);

      ModelFitResult fit_result = model_fit_facade.Fit();
      double chi2 = fit_result.getFinalEstimatorValue();
      std::cout << "chi2 of sample: " << chi2 << std::endl;

      std::stringstream hs;
      hs << "reldiff_" << fit_range.range_low << "-" << fit_range.range_high;

      if (histograms.find(fit_range.range_low) == histograms.end())
        histograms[fit_range.range_low] =
            new TH1D(hs.str().c_str(), hs.str().c_str(), 100, -6, 6);
      histograms[fit_range.range_low]->Fill(
          100.0 *
          (fit_result.getFitParameter("gauss_amplitude").value - refamp) /
          refamp);

      // root histograms
      std::stringstream hsr;
      hsr << "root_reldiff_" << fit_range.range_low << "-"
          << fit_range.range_high;
      if (root_histograms.find(fit_range.range_low) == root_histograms.end())
        root_histograms[fit_range.range_low] =
            new TH1D(hsr.str().c_str(), hsr.str().c_str(), 100, -6, 6);

      TF1 *fa = new TF1("mygauss", mygauss, fit_range.range_low,
                        fit_range.range_high, 1);
      fa->SetParameter(0, refamp);
      TFitResultPtr rootfitresult = gaushist->Fit(fa, "LRS");
      std::cout << "root chi2 is: " << rootfitresult->Chi2() << std::endl;
      root_histograms[fit_range.range_low]->Fill(
          100.0 *
          (rootfitresult->Parameter(0) / gaushist->GetBinWidth(1) - refamp) /
          refamp);
    }
  }

  std::stringstream ss;
  ss << "framework_test.root";

  TFile *output = new TFile(ss.str().c_str(), "RECREATE");

  gaushist->Write();

  std::cout << "writing " << histograms.size() << " histograms to file! "
            << std::endl;
  for (std::map<double, TH1D *>::iterator histogram_iter = histograms.begin();
       histogram_iter != histograms.end(); histogram_iter++) {
    histogram_iter->second->Write();
  }

  for (std::map<double, TH1D *>::iterator histogram_iter =
           root_histograms.begin();
       histogram_iter != root_histograms.end(); histogram_iter++) {
    histogram_iter->second->Write();
  }

  TGraphErrors *root_graph = createGraph(root_histograms);
  TGraphErrors *graph = createGraph(histograms);

  root_graph->Write("root_graph");
  graph->Write("graph");

  output->Close();
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "-n [number of events to generate]" << std::endl;
}

int main(int argc, char *argv[]) {
  int num_events = 100000;
  int c;

  while ((c = getopt(argc, argv, "hn:")) != -1) {
    switch (c) {
    case 'n':
      num_events = atoi(optarg);
      break;
    case '?':
      if (optopt == 'p' || optopt == 'm' || optopt == 'n')
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

  checkFramework(num_events);

  return 0;
}
