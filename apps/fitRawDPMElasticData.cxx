/*
 * fitRawDPMElasticData.cxx
 *
 *  Created on: Nov 7, 2013
 *      Author: steve
 */

#include "fit/ModelFitFacade.h"
#include "fit/estimatorImpl/Chi2Estimator.h"
#include "fit/estimatorImpl/LogLikelihoodEstimator.h"
#include "fit/estimatorImpl/UnbinnedLogLikelihoodEstimator.h"
#include "fit/data/ROOT/ROOTDataHelper.h"
#include "fit/minimizerImpl/ROOT/ROOTMinimizer.h"
#include "fit/data/DataStructs.h"
#include "visualization/ROOT/ROOTPlotter.h"
#include "visualization/ModelVisualizationProperties1D.h"
#include "model/PndLmdModelFactory.h"
#include "fit/PndLmdLumiFitResult.h"
#include "ROOTPlotHelper.hpp"
#include "data/PndLmdAngularData.h"
#include "ui/PndLmdFitFacade.h"

#include "tools/PbarPElasticScatteringEventGenerator.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>

#include "boost/property_tree/ptree.hpp"

#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "TChain.h"
#include "TClonesArray.h"
#include "TParticle.h"
#include "TDatabasePDG.h"
#include "TMath.h"
#include "TH1D.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TLatex.h"

#include "pnddata/PndMCTrack.h"
#include "detectors/lmd/LmdQA/PndLmdTrackQ.h"

struct data_options {
  double momentum;
  double theta_bound_low_in_mrad;
  double theta_bound_high_in_mrad;
  double theta_fit_range_low_in_mrad;
  double theta_fit_range_high_in_mrad;
  double phi_bound_low;
  double phi_bound_high;
  double theta_binning;
  double phi_binning;

  double tilt_x;
  double tilt_y;

  double elastic_cross_section;
};

struct hist_bunch {
  TH1D* t_hist;
  TH1D* th_hist;
  TH2D* th_phi_hist;
  std::shared_ptr<Data> unbinned_data;

  double gen_lumi;
  hist_bunch() :
      unbinned_data(new Data(2)) {
  }
};

hist_bunch createHistogramsFromRawDPMData(data_options &data_opt, unsigned int num_events) {
  hist_bunch return_histgrams;

  std::pair<TTree*, double> events = PbarPElasticScattering::generateEvents(data_opt.momentum,
      num_events, data_opt.theta_bound_low_in_mrad, data_opt.theta_bound_high_in_mrad, 1234);

  data_opt.elastic_cross_section = events.second;

  TClonesArray* fEvt = new TClonesArray("TParticle", 100);
  TTree &tree(*events.first);
  tree.SetBranchAddress("Particles", &fEvt);

  std::cout << "tree has " << tree.GetEntries() << " entries" << std::endl;
  unsigned int num_events_to_process = tree.GetEntries();
  if (num_events != -1 && num_events < num_events_to_process)
    num_events_to_process = num_events;

  PndLmdModelFactory model_factory;
  double t_bound_low = model_factory.getMomentumTransferFromTheta(data_opt.momentum,
      data_opt.theta_bound_low_in_mrad / 1000.0);
  double t_bound_high = model_factory.getMomentumTransferFromTheta(data_opt.momentum,
      data_opt.theta_bound_high_in_mrad / 1000.0);

  // create histograms from raw dpm files
  return_histgrams.th_hist = new TH1D("hist_dpm_theta", "", data_opt.theta_binning,
      data_opt.theta_bound_low_in_mrad / 1000.0, data_opt.theta_bound_high_in_mrad / 1000.0);
  return_histgrams.t_hist = new TH1D("hist_dpm_t", "", data_opt.theta_binning, t_bound_low,
      t_bound_high);
  return_histgrams.th_phi_hist = new TH2D("hist_dpm_theta_phi", "", data_opt.theta_binning,
      data_opt.theta_bound_low_in_mrad / 1000.0, data_opt.theta_bound_high_in_mrad / 1000.0,
      data_opt.phi_binning, data_opt.phi_bound_low, data_opt.phi_bound_high);

  std::cout << "Processing " << num_events_to_process << " events!" << std::endl;

  return_histgrams.gen_lumi = num_events_to_process / data_opt.elastic_cross_section;

  TDatabasePDG *pdg = TDatabasePDG::Instance();
  TLorentzVector ingoing(0, 0, data_opt.momentum,
      TMath::Sqrt(
          TMath::Power(data_opt.momentum, 2.0)
              + TMath::Power(pdg->GetParticle(-2212)->Mass(), 2.0)));

  TVector3 tilt(tan(data_opt.tilt_x), tan(data_opt.tilt_y), 1.0);
  TVector3 zprime = tilt.Unit();
  TVector3 yprime(-zprime.Y(), zprime.X(), 0.0);
  TVector3 xprime = yprime.Cross(zprime);
  double beta = acos(zprime.Z());
  double alpha = atan2(zprime.X(), -zprime.Y());
  double gamma = atan2(xprime.Z(), yprime.Z());

  for (unsigned int i = 0; i < num_events_to_process; i++) {
    tree.GetEntry(i);
    for (unsigned int np = 0; np < fEvt->GetEntries(); np++) {
      TParticle *particle = (TParticle*) fEvt->At(np);
      if (particle->GetPdgCode() == -2212) {
        TLorentzVector outgoing;
        particle->Momentum(outgoing);
        TVector3 measured_direction(outgoing.Vect());

        // rotating it myself
        if (false) {
          measured_direction.RotateZ(alpha);
          measured_direction.Rotate(beta, yprime);
          measured_direction.Rotate(gamma, zprime);
        }
        // if data should be rotated like in pandaroot
        //if (true)
        //  measured_direction.RotateUz(zprime);

        return_histgrams.t_hist->Fill(-1.0 * (outgoing - ingoing).M2());
        return_histgrams.th_hist->Fill(measured_direction.Theta());
        return_histgrams.th_phi_hist->Fill(measured_direction.Theta(), measured_direction.Phi());
      }
    }
  }
  return return_histgrams;
}

/*
 hist_bunch createHistogramsFromPandarootMCData(std::string data_path, data_options &data_opt,
 bool is_filelist_path, int index, int num_events) {
 hist_bunch return_histgrams;

 TChain tree("pndsim");

 if (is_filelist_path) {
 std::stringstream ss;
 ss << data_path << "/filelist_" << index << ".txt";
 std::cout << "Opening " << ss.str() << std::endl;
 std::ifstream infile(ss.str().c_str());
 std::string line;
 while (std::getline(infile, line)) {
 std::istringstream iss(line);

 std::cout << "Adding " << line.c_str() << std::endl;
 tree.Add(line.c_str());
 }
 } else {
 std::stringstream ss;
 if (index != -1) {
 ss << data_path << "/*_" << index << ".root";
 tree.Add(ss.str().c_str());
 } else {
 ss << data_path << "/*.root";
 tree.Add(ss.str().c_str());
 }
 }

 TClonesArray* fEvt = new TClonesArray("PndLmdTrackQ", 100);
 tree.SetBranchAddress("LMDTrackQ", &fEvt);

 std::cout << "tree has " << tree.GetEntries() << " entries" << std::endl;
 unsigned int num_events_to_process = tree.GetEntries();
 if (num_events != -1 && num_events < num_events_to_process)
 num_events_to_process = num_events;

 PndLmdModelFactory model_factory;
 double t_bound_low = model_factory.getMomentumTransferFromTheta(data_opt.momentum,
 data_opt.theta_bound_low);
 double t_bound_high = model_factory.getMomentumTransferFromTheta(data_opt.momentum,
 data_opt.theta_bound_high);

 // create histograms from raw dpm files
 return_histgrams.th_hist = new TH1D("hist_dpm_theta", "", data_opt.theta_binning,
 data_opt.theta_bound_low, data_opt.theta_bound_high);
 return_histgrams.t_hist = new TH1D("hist_dpm_t", "", data_opt.theta_binning, t_bound_low,
 t_bound_high);
 return_histgrams.th_phi_hist = new TH2D("hist_dpm_theta_phi", "", data_opt.theta_binning,
 data_opt.theta_bound_low, data_opt.theta_bound_high, data_opt.phi_binning,
 data_opt.phi_bound_low, data_opt.phi_bound_high);

 std::cout << "Processing " << num_events_to_process << " events!" << std::endl;

 return_histgrams.gen_lumi = num_events_to_process / data_opt.elastic_cross_section;

 TDatabasePDG *pdg = TDatabasePDG::Instance();
 TLorentzVector ingoing(0, 0, data_opt.momentum,
 TMath::Sqrt(
 TMath::Power(data_opt.momentum, 2.0)
 + TMath::Power(pdg->GetParticle(-2212)->Mass(), 2.0)));

 TVector3 tilt(tan(data_opt.tilt_x), tan(data_opt.tilt_y), 1.0);
 TVector3 zprime = tilt.Unit();
 TVector3 yprime(-zprime.Y(), zprime.X(), 0.0);
 TVector3 xprime = yprime.Cross(zprime);
 double beta = acos(zprime.Z());
 double alpha = atan2(zprime.X(), -zprime.Y());
 double gamma = atan2(xprime.Z(), yprime.Z());

 DataPointProxy dpp;

 for (unsigned int i = 0; i < num_events_to_process; i++) {
 tree.GetEntry(i);
 for (unsigned int np = 0; np < fEvt->GetEntries(); np++) {
 PndLmdTrackQ *particle = (PndLmdTrackQ*) fEvt->At(np);
 if (particle->GetPDGcode() == -2212) {
 TVector3 measured_direction(0.0, 0.0, 1.0);
 measured_direction.SetMagThetaPhi(particle->GetMCmom(), particle->GetMCtheta(),
 particle->GetMCphi());
 if (false) {
 measured_direction.Rotate(-gamma, zprime);
 measured_direction.Rotate(-beta, yprime);
 measured_direction.RotateZ(-alpha);
 }
 //return_histgrams.t_hist->Fill(-1.0 * (outgoing - ingoing).M2());
 return_histgrams.th_hist->Fill(measured_direction.Theta());
 return_histgrams.th_phi_hist->Fill(measured_direction.Theta(), measured_direction.Phi());

 if (measured_direction.Theta() > data_opt.theta_bound_low
 && measured_direction.Theta() < data_opt.theta_bound_high) {
 std::shared_ptr<DataStructs::binned_data_point> bdp(new DataStructs::binned_data_point);
 dpp.setBinnedDataPoint(bdp);
 dpp.setPointUsed(true);
 return_histgrams.unbinned_data->insertData(dpp);
 }
 }
 }
 }
 return return_histgrams;
 }*/

void fit1D(const hist_bunch &histograms, data_options &data_opt, std::string filename) {
  DataStructs::DimensionRange fit_range_th(data_opt.theta_fit_range_low_in_mrad / 1000.0,
      data_opt.theta_fit_range_high_in_mrad / 1000.0);

  TH1D* hist(histograms.th_hist);

  // create 1d model and initialize tilt parameters
  PndLmdModelFactory model_factory;

  boost::property_tree::ptree fit_op_full;
  fit_op_full.put("fit_dimension", 1);
  fit_op_full.put("dpm_elastic_parts", "ALL");
  fit_op_full.put("theta_t_trafo_option", "APPROX");
  fit_op_full.put("momentum_transfer_active", false);
  fit_op_full.put("acceptance_correction_active", false);
  fit_op_full.put("resolution_smearing_active", false);
  fit_op_full.put("divergence_smearing_active", false);

  fit_op_full.put("fix_beam_tilts", true);
  fit_op_full.put("beam_tilt_x", 0.0);
  fit_op_full.put("beam_tilt_y", 0.0);

  PndLmdAngularData tempdata;
  tempdata.setLabMomentum(data_opt.momentum);

  std::shared_ptr<Model> model = model_factory.generateModel(fit_op_full, tempdata);
  if (model->init()) {
    std::cout << "Error: not all parameters have been set!" << std::endl;
  }

  /*model->getModelParameterSet().freeModelParameter("tilt_x");
   model->getModelParameterSet().freeModelParameter("tilt_y");
   model->getModelParameterSet().setModelParameterValue("tilt_x", 0.0);
   model->getModelParameterSet().setModelParameterValue("tilt_y", 0.0);*/

  //model->getModelParameterSet().printInfo();
  // integral - just for testing purpose
  std::vector<DataStructs::DimensionRange> int_range;
  int_range.push_back(fit_range_th);
  //int_range.push_back(plot_range_phi);

  PndLmdFitFacade lmd_helper;
  double integral_data = 0.0;
  std::cout << "calculating data integral..." << std::endl;
  integral_data = lmd_helper.calcHistIntegral(hist, int_range);

  double integral_func = model->Integral(int_range, 1e-3);
  std::cout << "integral: " << integral_func << std::endl;

  double lumi_start = integral_data / integral_func / hist->GetXaxis()->GetBinWidth(1);
  std::cout << "Using start luminosity: " << lumi_start << std::endl;
  model->getModelParameterSet().freeModelParameter("luminosity");
  model->getModelParameterSet().setModelParameterValue("luminosity", lumi_start);

  ModelFitFacade fit_facade;

  std::shared_ptr<LogLikelihoodEstimator> loglikelihood_est(new LogLikelihoodEstimator());
  fit_facade.setEstimator(loglikelihood_est);
  std::shared_ptr<Data> data(new Data(1));
  ROOTDataHelper data_helper;
  data_helper.fillBinnedData(data, hist);
  fit_facade.setData(data);

  fit_facade.setModel(model);

  // create minimizer instance with control parameter
  std::shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());

  fit_facade.setMinimizer(minuit_minimizer);

  EstimatorOptions est_opt;
  est_opt.setWithIntegralScaling(false);
  est_opt.setFitRangeX(fit_range_th);
  //est_opt.setFitRangeY(plot_range_phi);
  fit_facade.setEstimatorOptions(est_opt);

  ModelFitResult fit_result = fit_facade.Fit();

  ROOTPlotter root_plotter;

  ModelVisualizationProperties1D vis_prop_th;
  vis_prop_th.setEvaluations(data_opt.theta_binning);
  vis_prop_th.setBinningFactor(hist->GetXaxis()->GetBinWidth(1));
  vis_prop_th.setPlotRange(fit_range_th);

  TGraphAsymmErrors* graph = root_plotter.createGraphFromModel1D(model, vis_prop_th);

  double lumi = fit_result.getFitParameter("luminosity").value;
  double lumi_err = fit_result.getFitParameter("luminosity").error;
  double lumi_ref = histograms.gen_lumi;

  std::pair<double, double> lumi_reldiff = std::make_pair(100.0 * (lumi - lumi_ref) / lumi_ref,
      100.0 * lumi_err / lumi_ref);

  std::cout << "lumi: " << lumi << " lumi_ref: " << lumi_ref << std::endl;
  std::cout << "accuracy: " << (lumi - lumi_ref) / lumi_ref << std::endl;

  NeatPlotting::GraphAndHistogramHelper neat_plot_helper;
  NeatPlotting::PlotBundle residual_plot_bundle;
  TGraphAsymmErrors *residual = neat_plot_helper.makeDifferenceGraph(hist, graph);

  TFile file("fitresult.root", "RECREATE");
  residual->Write("residual");
  file.Close();
}

void fit2D(hist_bunch histograms, data_options &data_opt, bool binned = true) {
  DataStructs::DimensionRange plot_range_th(data_opt.theta_bound_low_in_mrad / 1000.0,
      data_opt.theta_bound_high_in_mrad / 1000.0);
  DataStructs::DimensionRange plot_range_phi(data_opt.phi_bound_low, data_opt.phi_bound_high);

  // create 2d model and initialize tilt parameters
  PndLmdModelFactory model_factory;

  boost::property_tree::ptree fit_op_full;
  fit_op_full.put("fit_dimension", 2);
  fit_op_full.put("acceptance_correction_active", false);
  fit_op_full.put("resolution_smearing_active", false);

  PndLmdAngularData tempdata;
  tempdata.setLabMomentum(data_opt.momentum);

  std::shared_ptr<Model> model = model_factory.generateModel(fit_op_full, tempdata);
  if (model->init()) {
    std::cout << "Error: not all parameters have been set!" << std::endl;
  }

  model->getModelParameterSet().freeModelParameter("tilt_x");
  model->getModelParameterSet().freeModelParameter("tilt_y");
  /*model->getModelParameterSet().setModelParameterValue("tilt_x",
   data_opt.tilt_x);
   model->getModelParameterSet().setModelParameterValue("tilt_y",
   data_opt.tilt_y);
   model->getModelParameterSet().getModelParameter("tilt_x")->setParameterFixed(
   true);
   model->getModelParameterSet().getModelParameter("tilt_y")->setParameterFixed(
   true);*/

  //model->getModelParameterSet().printInfo();
  // integral - just for testing purpose
  std::vector<DataStructs::DimensionRange> int_range;
  int_range.push_back(plot_range_th);
  int_range.push_back(plot_range_phi);

  PndLmdFitFacade lmd_helper;
  double integral_data = 0.0;
  std::cout << "calculating data integrals..." << std::endl;
  integral_data = lmd_helper.calcHistIntegral(histograms.th_phi_hist, int_range);
  std::cout << "integral: " << integral_data << std::endl;
  double integral_func = model->Integral(int_range, 1e-3);
  std::cout << "integral: " << integral_func << std::endl;

  ModelFitFacade fit_facade;

  if (binned) {
    std::shared_ptr<LogLikelihoodEstimator> loglikelihood_est(new LogLikelihoodEstimator());
    fit_facade.setEstimator(loglikelihood_est);

    std::shared_ptr<Data> data(new Data(2));
    ROOTDataHelper data_helper;
    data_helper.fillBinnedData(data, histograms.th_phi_hist);
    fit_facade.setData(data);

    double lumi_start = integral_data / integral_func
        / histograms.th_phi_hist->GetXaxis()->GetBinWidth(1)
        / histograms.th_phi_hist->GetYaxis()->GetBinWidth(1);

    std::cout << "Using start luminosity: " << lumi_start << std::endl;
    model->getModelParameterSet().freeModelParameter("luminosity");
    model->getModelParameterSet().setModelParameterValue("luminosity", 1163.33);
  } else {
    std::shared_ptr<UnbinnedLogLikelihoodEstimator> loglikelihood_est(
        new UnbinnedLogLikelihoodEstimator());
    fit_facade.setEstimator(loglikelihood_est);
    fit_facade.setData(histograms.unbinned_data);

    double lumi_start = integral_data / integral_func;
    std::cout << "Using start luminosity: " << lumi_start << std::endl;
    model->getModelParameterSet().freeModelParameter("luminosity");
    model->getModelParameterSet().setModelParameterValue("luminosity", 5700.33); // change this to num events or something...
    model->getModelParameterSet().getModelParameter("luminosity")->setParameterFixed(true);
  }

  fit_facade.setModel(model);

  // create minimizer instance with control parameter
  std::shared_ptr<ROOTMinimizer> minuit_minimizer(new ROOTMinimizer());

  fit_facade.setMinimizer(minuit_minimizer);

  EstimatorOptions est_opt;
  est_opt.setWithIntegralScaling(false);
  est_opt.setFitRangeX(plot_range_th);
  est_opt.setFitRangeY(plot_range_phi);
  fit_facade.setEstimatorOptions(est_opt);

  ModelFitResult fit_result = fit_facade.Fit();

  ROOTPlotter root_plotter;

  ModelVisualizationProperties1D vis_prop_th;
  vis_prop_th.setEvaluations(data_opt.theta_binning);
  vis_prop_th.setBinningFactor(histograms.th_phi_hist->GetXaxis()->GetBinWidth(1));
  vis_prop_th.setPlotRange(plot_range_th);

  ModelVisualizationProperties1D vis_prop_phi;
  vis_prop_phi.setEvaluations(data_opt.phi_binning);
  vis_prop_phi.setBinningFactor(histograms.th_phi_hist->GetYaxis()->GetBinWidth(1));
  vis_prop_phi.setPlotRange(plot_range_phi);

  std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D> vis_prop_pair =
      std::make_pair(vis_prop_th, vis_prop_phi);

  /*integral_func = model->Integral(int_range, 1e-3);
   std::cout << "integral: " << integral_func << std::endl;

   lumi_start = integral_data / integral_func
   / pr_mc_histograms.th_phi_hist->GetXaxis()->GetBinWidth(1)
   / pr_mc_histograms.th_phi_hist->GetYaxis()->GetBinWidth(1);

   model->getModelParameterSet().freeModelParameter("luminosity");
   model->getModelParameterSet().setModelParameterValue("luminosity",
   lumi_start);*/

  TH2D* model_hist = root_plotter.createHistogramFromModel2D(model, vis_prop_pair);

  NeatPlotting::GraphAndHistogramHelper hist_helper;
  TH2D* diffmodel = hist_helper.makeRatioHistogram(model_hist, histograms.th_phi_hist);

  TCanvas c;

  double lumi = fit_result.getFitParameter("luminosity").value;
  double lumi_err = fit_result.getFitParameter("luminosity").error;
  double lumi_ref = histograms.gen_lumi;

  std::pair<double, double> lumi_reldiff = std::make_pair(100.0 * (lumi - lumi_ref) / lumi_ref,
      100.0 * lumi_err / lumi_ref);

  diffmodel->Draw("colz");
  diffmodel->GetXaxis()->SetTitle("#theta / rad");
  diffmodel->GetYaxis()->SetTitle("#phi / rad");
  diffmodel->GetZaxis()->SetRangeUser(0.5, 1.5);
  std::stringstream titletext;
  titletext << "tilt_{x}=" << data_opt.tilt_x << " rad, tilt_{y}=" << data_opt.tilt_y << " rad  | "
      << lumi_reldiff.first << " +- " << lumi_reldiff.second;
  diffmodel->SetTitle(titletext.str().c_str());

  c.SaveAs("tilttest_2d.pdf");
}

void fitRawDPMElasticData(double momentum, unsigned int num_events) {

  data_options data_opt;
  data_opt.momentum = momentum;
  data_opt.theta_bound_low_in_mrad = 2.7;
  data_opt.theta_bound_high_in_mrad = 13.0;
  data_opt.theta_fit_range_low_in_mrad = data_opt.theta_bound_low_in_mrad + 0.1;
  data_opt.theta_fit_range_high_in_mrad = data_opt.theta_bound_high_in_mrad - 0.1;
  data_opt.phi_bound_low = -TMath::Pi();
  data_opt.phi_bound_high = TMath::Pi();
  data_opt.theta_binning = 200;
  data_opt.phi_binning = 50;
  data_opt.tilt_x = 0.0;
  data_opt.tilt_y = 0.0;
  data_opt.elastic_cross_section = 0.0;

  hist_bunch dpm_mc_histograms = createHistogramsFromRawDPMData(data_opt, num_events);

  /*hist_bunch pr_mc_histograms = createHistogramsFromPandarootMCData(data_path, data_opt,
   is_filelist_path, index, num_events);*/

  /*NeatPlotting::GraphAndHistogramHelper hist_helper;
   TH2D* diff = hist_helper.makeRatioHistogram(dpm_mc_histograms.th_phi_hist,
   pr_mc_histograms.th_phi_hist);

   TCanvas c;
   diff->Draw("colz");
   c.SaveAs("tilttest.pdf");*/

  fit1D(dpm_mc_histograms, data_opt, "raw_dpm_fit1d.pdf");
  //fit1D(pr_mc_histograms, data_opt, "pandaroot_mc_fit1d.pdf");
  //fit2D(dpm_mc_histograms, data_opt);
}

void displayInfo() {
// display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "-m [pbar momentum]" << std::endl;
  std::cout << "-n [number of events to process]" << std::endl;

}

int main(int argc, char* argv[]) {
  bool is_mom_set = false, is_num_events_set = false;
  double momentum = -1.0;
  unsigned int num_events = 0;
  int c;

  while ((c = getopt(argc, argv, "hm:n:")) != -1) {
    switch (c) {
      case 'm':
        momentum = atof(optarg);
        is_mom_set = true;
        break;
      case 'n':
        num_events = atoi(optarg);
        is_num_events_set = true;
        break;
      case '?':
        if (optopt == 'm' || optopt == 'n')
          std::cerr << "Option -" << optopt << " requires an argument." << std::endl;
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

  if (is_mom_set && is_num_events_set) {
    fitRawDPMElasticData(momentum, num_events);
  } else
    displayInfo();

  return 0;
}
