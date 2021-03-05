/*
 * PndLmdModelFactory.cxx
 *
 *  Created on: Dec 18, 2012
 *      Author: steve
 */

#include "PndLmdModelFactory.h"
#include "AsymmetricGaussianModel1D.h"
#include "CachedModel2D.h"
#include "PndLmdDPMAngModel1D.h"
#include "PndLmdDPMAngModel2D.h"
#include "PndLmdDPMMTModel1D.h"
#include "PndLmdDivergenceSmearingModel2D.h"
#include "PndLmdFastDPMAngModel2D.h"
#include "PndLmdROOTDataModel1D.h"
#include "PndLmdROOTDataModel2D.h"
#include "data/PndLmdAcceptance.h"
#include "fit/ModelFitResult.h"
#include "fit/PndLmdLumiFitResult.h"
#include "models1d/DoubleGaussianModel1D.h"
#include "models1d/GaussianModel1D.h"
#include "models2d/DataModel2D.h"
#include "models2d/GaussianModel2D.h"
#include "ui/PndLmdRuntimeConfiguration.h"

#include "PndLmdBackgroundModel1D.h"
#include "PndLmdSignalBackgroundModel1D.h"

#include "PndLmdDPMModelParametrization.h"
#include "model/PndLmdDifferentialSmearingConvolutionModel2D.h"
#include "model/PndLmdSmearingConvolutionModel2D.h"
#include "operators1d/ProductModel1D.h"
#include "operators1d/convolution/NumericConvolutionModel1D.h"
#include "operators1d/convolution/SmearingConvolutionModel1D.h"
#include "operators2d/ProductModel2D.h"
#include "operators2d/integration/SimpleIntegralStrategy2D.h"

#include "data/PndLmdAngularData.h"

#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TH2.h"

PndLmdModelFactory::PndLmdModelFactory() {}

PndLmdModelFactory::~PndLmdModelFactory() {}

std::shared_ptr<PndLmdSmearingModel2D>
PndLmdModelFactory::generate2DSmearingModel(
    const LumiFit::LmdDimension &dimx,
    const LumiFit::LmdDimension &dimy) const {
  // convert to a vector for faster access due to better caching
  std::cout << "converting detector smearing contributions to vector for fast "
               "access..."
            << std::endl;

  // TH2D histx("ax", "", 200, -0.013, 0.013, 200, -0.013, 0.013);
  // TH2D histy("ay", "", 200, -0.013, 0.013, 200, -0.013, 0.013);

  /*TTree *tree = (TTree*)resolution_map_data.getDataTree()->Clone("asdf");
   Point2D mc;
   Point2D reco;
   tree->SetBranchAddress("reco_x", &reco.x);
   tree->SetBranchAddress("reco_y", &reco.y);
   tree->SetBranchAddress("mc_x", &mc.x);
   tree->SetBranchAddress("mc_y", &mc.y);

   std::map<Point2D, Point2DCloud> prefill_map;

   for (unsigned int i = 0; i < tree->GetEntries(); ++i) {
   tree->GetEntry(i);

   ++prefill_map[reco].points[mc];
   ++prefill_map[reco].total_count;
   }*/

  const auto &hit_map = resolution_map_data.getHitMap();

  std::cout << "hit map size: " << hit_map.size() << std::endl;

  std::map<Point2D, std::map<Point2D, double>> temp_map;

  unsigned long average_contributors(0);
  for (auto const &mc_bin : hit_map) {
    unsigned int overall_count(0);
    average_contributors += mc_bin.second.points.size();
    for (auto const &reco_bin_item : mc_bin.second.points) {
      temp_map[reco_bin_item.first][mc_bin.first] +=
          (1.0 * reco_bin_item.second) / mc_bin.second.total_count;

      overall_count += reco_bin_item.second;
    }
    if (overall_count != mc_bin.second.total_count)
      std::cout << "overall_count missmatch! (should be "
                << mc_bin.second.total_count << "): " << overall_count
                << std::endl;
  }
  std::cout << "average reco bins per mc bin: "
            << 1.0 * average_contributors / hit_map.size() << std::endl;

  std::vector<RecoBinSmearingContributions> smearing_param;
  smearing_param.reserve(temp_map.size());

  average_contributors = 0;
  for (auto const &reco_bin : temp_map) {
    if (reco_bin.first.x < dimx.dimension_range.getRangeLow())
      continue;
    if (reco_bin.first.x > dimx.dimension_range.getRangeHigh())
      continue;
    if (reco_bin.first.y < dimy.dimension_range.getRangeLow())
      continue;
    if (reco_bin.first.y > dimy.dimension_range.getRangeHigh())
      continue;
    RecoBinSmearingContributions rbsc;
    rbsc.reco_bin_x = reco_bin.first.x;
    rbsc.reco_bin_y = reco_bin.first.y;

    rbsc.contributor_coordinate_weight_list.reserve(reco_bin.second.size());

    double overall_smear(0.0);
    average_contributors += reco_bin.second.size();
    // std::cout<<"size: "<<reco_bin.second.size()<<std::endl;
    for (auto const &contribution_list_item : reco_bin.second) {
      ContributorCoordinateWeight cw;
      cw.bin_center_x = contribution_list_item.first.x;
      cw.bin_center_y = contribution_list_item.first.y;

      // cw.area = resolution_map_data.getPrimaryDimension().bin_size
      // * resolution_map_data.getSecondaryDimension().bin_size;
      cw.smear_weight = contribution_list_item.second;
      rbsc.contributor_coordinate_weight_list.push_back(cw);

      overall_smear += cw.smear_weight;
      // histx.Fill(rbsc.reco_bin_x - cw.bin_center_x,
      //    rbsc.reco_bin_y - cw.bin_center_y);
      //   histy.Fill(rbsc.reco_bin_y, cw.bin_center_y);
      // std::cout << rbsc.reco_bin_x << " : " << rbsc.reco_bin_y << " -> "
      // << cw.bin_center_x << " : " << cw.bin_center_y << " = "
      // << cw.smear_weight << std::endl;
    }
    smearing_param.push_back(rbsc);
    // std::cout << "smear weight: " << overall_smear << std::endl;
  }
  std::cout << "average mc bins per reco bin: "
            << 1.0 * average_contributors / temp_map.size() << std::endl;
  std::cout << "done!" << std::endl;

  // TVirtualPad *current_pad = gPad;
  // TCanvas can;
  // histx.Draw("colz");
  // can.Update();
  // can.SaveAs("corrx.pdf");
  // histy.Draw("colz");
  // can.Update();
  // can.SaveAs("corry.pdf");

  // gPad = current_pad; // reset pad to the one before*/

  std::shared_ptr<PndLmdSmearingModel2D> detector_smearing_model(
      new PndLmdSmearingModel2D(dimx, dimy));

  detector_smearing_model->setSmearingParameterization(smearing_param);
  // detector_smearing_model->setSearchDistances(
  //    resolution_map_data.getPrimaryDimension().bin_size / 2,
  //    resolution_map_data.getSecondaryDimension().bin_size / 2);
  return detector_smearing_model;
}

std::shared_ptr<PndLmdSmearingModel2D>
PndLmdModelFactory::generate2DSmearingModel(
    const PndLmdHistogramData &data, const LumiFit::LmdDimension &dimx,
    const LumiFit::LmdDimension &dimy) const {
  std::map<std::pair<double, double>, std::map<LumiFit::BinDimension, double>>
      resolution_map;

  TH2D *recohist2d = data.get2DHistogram();

  int prim_dim_bins = data.getPrimaryDimension().bins;
  double prim_dim_binsize = data.getPrimaryDimension().bin_size;
  double prim_dim_start =
      data.getPrimaryDimension().dimension_range.getRangeLow();

  int sec_dim_bins = data.getSecondaryDimension().bins;
  double sec_dim_binsize = data.getSecondaryDimension().bin_size;
  double sec_dim_start =
      data.getSecondaryDimension().dimension_range.getRangeLow();

  double binning_factor = 1 / sec_dim_binsize / prim_dim_binsize;

  std::cout << "calculating detector smearing contributions from "
            << resolutions.size() << " objects..." << std::endl;

  // loop over all resolution objects
  for (unsigned int i = 0; i < resolutions.size(); i++) {
    // std::cout << "processing resolution " << i << std::endl;
    if (resolutions[i].getSecondaryDimension().is_active) {
      // try to find the selction dimension (mc values)
      LumiFit::LmdDimension prim_dim;
      LumiFit::LmdDimension sec_dim;

      bool found_primary_selection(false);
      bool found_secondary_selection(false);

      const std::set<LumiFit::LmdDimension> &selection_set =
          resolutions[i].getSelectorSet();
      std::set<LumiFit::LmdDimension>::const_iterator selection_set_it;
      for (selection_set_it = selection_set.begin();
           selection_set_it != selection_set.end(); selection_set_it++) {
        if (selection_set_it->dimension_options.dimension_type ==
            data.getPrimaryDimension().dimension_options.dimension_type) {
          prim_dim = *selection_set_it;
          found_primary_selection = true;
        } else if (selection_set_it->dimension_options.dimension_type ==
                   data.getSecondaryDimension()
                       .dimension_options.dimension_type) {
          sec_dim = *selection_set_it;
          found_secondary_selection = true;
        }
      }

      if (found_primary_selection && found_secondary_selection) {
        // ok we found the selection dimensions
        TH2D *hist2d = resolutions[i].get2DHistogram();
        // calculated normalization integral
        double hist_integral = hist2d->Integral();
        if (hist_integral <= 0.0)
          continue;

        double total_smearing_probability = 0.0;
        double total_smearing_probability_noweight = 0.0;
        int counter = 0;

        // loop over the resolution histogram bins
        for (unsigned int ibinx = 1; ibinx <= hist2d->GetNbinsX(); ++ibinx) {
          for (unsigned int ibiny = 1; ibiny <= hist2d->GetNbinsY(); ++ibiny) {
            // ok so now we just need to find the reco bins (there could be some
            // overlap with multiple reco bins) for each of the resolution bins
            // and weight the full bin probability according to the overlap
            if (hist2d->GetBinContent(ibinx, ibiny) > 0.0) {
              double smearing_probability =
                  hist2d->GetBinContent(ibinx, ibiny) / hist_integral *
                  binning_factor;

              // total_smearing_probability_noweight += smearing_probability
              //		/ binning_factor;

              // std::cout << "smearing probability: " << smearing_probability
              //<< std::endl;

              LumiFit::BinDimension reso_bin;
              reso_bin.x_range.range_low =
                  (prim_dim.dimension_range.getDimensionMean() +
                   hist2d->GetXaxis()->GetBinLowEdge(ibinx));
              reso_bin.x_range.range_high =
                  (prim_dim.dimension_range.getDimensionMean() +
                   hist2d->GetXaxis()->GetBinUpEdge(ibinx));
              reso_bin.y_range.range_low =
                  (sec_dim.dimension_range.getDimensionMean() +
                   hist2d->GetYaxis()->GetBinLowEdge(ibiny));
              reso_bin.y_range.range_high =
                  (sec_dim.dimension_range.getDimensionMean() +
                   hist2d->GetYaxis()->GetBinUpEdge(ibiny));

              LumiFit::BinDimension reco_bin;

              // calculated boundaries
              int recbinx_min = (reso_bin.x_range.range_low - prim_dim_start) /
                                    prim_dim_binsize +
                                1;
              int recbinx_max = (reso_bin.x_range.range_high - prim_dim_start) /
                                    prim_dim_binsize +
                                1;
              int recbiny_min = (reso_bin.y_range.range_low - sec_dim_start) /
                                    sec_dim_binsize +
                                1;
              int recbiny_max = (reso_bin.y_range.range_high - sec_dim_start) /
                                    sec_dim_binsize +
                                1;

              // in case the resolution bin is further down the lower bound of
              // the reco hist just skip

              /*if (recbinx_max <= 0 || recbinx_min >= prim_dim_bins)
               std::cout << recbinx_min << " " << recbinx_max << " "
               << prim_dim_bins << std::endl;
               if (recbiny_max <= 0 || recbiny_min >= sec_dim_bins)
               std::cout << recbiny_min << " " << recbiny_max << " "
               << sec_dim_bins << std::endl;*/

              if (recbinx_max <= 0 || recbinx_min >= prim_dim_bins)
                continue;
              if (recbiny_max <= 0 || recbiny_min >= sec_dim_bins)
                continue;

              /*std::cout << theta_start << " asdf " << phi_start << std::endl;
               std::cout << recbinx_min << " = " << reso_bin.x_min << " "
               << theta_start << " " << theta_binsize << std::endl;
               std::cout << recbinx_max << " = " << reso_bin.x_max << " "
               << theta_start << " " << theta_binsize << std::endl;
               std::cout << recbiny_min << " = " << reso_bin.y_min << " "
               << phi_start << " " << phi_binsize << std::endl;
               std::cout << recbiny_max << " = " << reso_bin.y_max << " "
               << phi_start << " " << phi_binsize << std::endl;*/

              for (unsigned int irecbinx = recbinx_min; irecbinx <= recbinx_max;
                   irecbinx++) {
                for (unsigned int irecbiny = recbiny_min;
                     irecbiny <= recbiny_max; irecbiny++) {
                  // calculate overlap with elastic data reco bins

                  reco_bin.x_range.range_low =
                      recohist2d->GetXaxis()->GetBinLowEdge(irecbinx);
                  reco_bin.x_range.range_high =
                      recohist2d->GetXaxis()->GetBinUpEdge(irecbinx);
                  reco_bin.y_range.range_low =
                      recohist2d->GetYaxis()->GetBinLowEdge(irecbiny);
                  reco_bin.y_range.range_high =
                      recohist2d->GetYaxis()->GetBinUpEdge(irecbiny);

                  double weight =
                      LumiFit::calculateBinOverlap(reco_bin, reso_bin);

                  if (weight < 1e-14)
                    continue;

                  double prim_var_rec =
                      recohist2d->GetXaxis()->GetBinCenter(irecbinx);
                  double sec_var_rec =
                      recohist2d->GetYaxis()->GetBinCenter(irecbiny);

                  // std::cout << "weight: " << weight << std::endl;

                  if (resolution_map[std::make_pair(prim_var_rec, sec_var_rec)]
                          .find(
                              LumiFit::BinDimension(prim_dim.dimension_range,
                                                    sec_dim.dimension_range)) ==
                      resolution_map[std::make_pair(prim_var_rec, sec_var_rec)]
                          .end()) {
                    resolution_map[std::make_pair(prim_var_rec, sec_var_rec)]
                                  [LumiFit::BinDimension(
                                      prim_dim.dimension_range,
                                      sec_dim.dimension_range)] =
                                      weight * smearing_probability;
                  } else {
                    resolution_map[std::make_pair(prim_var_rec, sec_var_rec)]
                                  [LumiFit::BinDimension(
                                      prim_dim.dimension_range,
                                      sec_dim.dimension_range)] +=
                        weight * smearing_probability;
                  }

                  total_smearing_probability +=
                      weight * smearing_probability / binning_factor;
                  counter++;
                }
              }
            }
          }
        }
        /*if (total_smearing_probability < 0.99) {
         std::cout << "probability for "
         << prim_dim.dimension_range.getDimensionMean() << ", "
         << sec_dim.dimension_range.getDimensionMean() << std::endl;
         std::cout << "total smearing probs: " << total_smearing_probability
         << " " << total_smearing_probability_noweight << std::endl;
         std::cout << counter << std::endl;
         }*/
      }
    }
  }

  /*std::map<PndLmdLumiHelper::bin_dimension, std::pair<double, int> > test_map;

   std::map<std::pair<double, double>,
   std::map<PndLmdLumiHelper::bin_dimension, double> >::iterator it;

   for (it = resolution_map.begin(); it != resolution_map.end(); it++) {
   std::map<PndLmdLumiHelper::bin_dimension, double>::iterator it2;
   for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
   if (test_map.find(it2->first) == test_map.end())
   test_map[it2->first] = std::make_pair(it2->second, 1);
   else {
   test_map[it2->first].first += it2->second;
   test_map[it2->first].second++;
   }
   }
   }

   std::map<PndLmdLumiHelper::bin_dimension, std::pair<double, int> >::iterator
   it2; for (it2 = test_map.begin(); it2 != test_map.end(); it2++) { std::cout
   << "probability for " << it2->first.x_range.range_low << ", "
   << it2->first.x_range.range_high << ": " << it2->second.second << " "
   << it2->second.first << std::endl;
   }*/

  // convert to a vector for faster access due to better caching
  std::cout << "converting detector smearing contributions to vector for fast "
               "access..."
            << std::endl;

  std::vector<RecoBinSmearingContributions> smearing_param;
  smearing_param.reserve(resolution_map.size());

  std::map<std::pair<double, double>,
           std::map<LumiFit::BinDimension, double>>::iterator iter;
  for (iter = resolution_map.begin(); iter != resolution_map.end(); ++iter) {
    RecoBinSmearingContributions rbsc;
    rbsc.reco_bin_x = iter->first.first;
    rbsc.reco_bin_y = iter->first.second;

    rbsc.contributor_coordinate_weight_list.reserve(iter->second.size());
    std::map<LumiFit::BinDimension, double>::iterator contribution_list_iter;
    for (contribution_list_iter = iter->second.begin();
         contribution_list_iter != iter->second.end();
         ++contribution_list_iter) {
      ContributorCoordinateWeight cw;
      cw.bin_center_x =
          contribution_list_iter->first.x_range.getDimensionMean();
      cw.bin_center_y =
          contribution_list_iter->first.y_range.getDimensionMean();

      //      cw.area =
      //      contribution_list_iter->first.x_range.getDimensionLength()
      //          * contribution_list_iter->first.y_range.getDimensionLength();
      cw.smear_weight = contribution_list_iter->second;
      rbsc.contributor_coordinate_weight_list.push_back(cw);
    }
    smearing_param.push_back(rbsc);
  }
  std::cout << "done!" << std::endl;

  std::shared_ptr<PndLmdSmearingModel2D> detector_smearing_model(
      new PndLmdSmearingModel2D(dimx, dimy));

  detector_smearing_model->setSmearingParameterization(smearing_param);
  return detector_smearing_model;
}

double PndLmdModelFactory::getMomentumTransferFromTheta(double plab,
                                                        double theta) const {
  PndLmdDPMAngModel1D model("dpm_angular_1d", LumiFit::ALL, LumiFit::APPROX);
  std::shared_ptr<Parametrization> para(
      new PndLmdDPMModelParametrization(model.getModelParameterSet()));
  model.getModelParameterHandler().registerParametrizations(
      model.getModelParameterSet(), para);
  model.getModelParameterSet().setModelParameterValue("p_lab", plab);
  ((Model1D *)&model)->init();
  return -model.getMomentumTransferFromTheta(theta);
}

std::shared_ptr<Model1D> PndLmdModelFactory::generate1DModel(
    const boost::property_tree::ptree &model_opt_ptree,
    const PndLmdAngularData &data) const {
  std::shared_ptr<Model1D> current_model;

  LumiFit::DPMElasticParts elastic_parts = LumiFit::StringToDPMElasticParts.at(
      model_opt_ptree.get<std::string>("dpm_elastic_parts"));

  LumiFit::TransformationOption trans_option =
      LumiFit::StringToTransformationOption.at(
          model_opt_ptree.get<std::string>("theta_t_trafo_option"));

  if (model_opt_ptree.get<bool>("momentum_transfer_active")) {
    current_model.reset(new PndLmdDPMMTModel1D("dpm_mt_1d", elastic_parts));
    // set free parameters
  } else {
    current_model.reset(
        new PndLmdDPMAngModel1D("dpm_angular_1d", elastic_parts, trans_option));
  }

  std::shared_ptr<Parametrization> dpm_parametrization(
      new PndLmdDPMModelParametrization(current_model->getModelParameterSet()));
  current_model->getModelParameterHandler().registerParametrizations(
      current_model->getModelParameterSet(), dpm_parametrization);

  if (model_opt_ptree.get<bool>(
          "acceptance_correction_active")) { // with acceptance corr
    if (acceptance.getAcceptance1D()) {
      // translate acceptance interpolation option
      PndLmdROOTDataModel1D *data_model =
          new PndLmdROOTDataModel1D("acceptance_1d");

      TVirtualPad *current_pad = gPad;
      TCanvas can;
      acceptance.getAcceptance1D()->Draw();
      can.Update();

      data_model->setGraph(acceptance.getAcceptance1D()->GetPaintedGraph());
      data_model->setIntpolType(LumiFit::StringToInterpolationType.at(
          model_opt_ptree.get<std::string>("acceptance_interpolation")));
      if (!model_opt_ptree.get<bool>(
              "automatic_acceptance_boundary_finding_active")) {
        data_model->setAcceptanceBounds(
            model_opt_ptree.get<double>("acceptance_bound_low"),
            model_opt_ptree.get<double>("acceptance_bound_high"));
      }
      data_model->setDataDimension(
          acceptance.getPrimaryDimension().dimension_range);
      std::shared_ptr<Model1D> acc(data_model);

      current_model.reset(
          new ProductModel1D("acceptance_corrected_1d", current_model, acc));

      gPad = current_pad; // reset pad to the one before
    } else {
      std::cout << "ERROR: requesting an acceptance corrected model without an "
                   "acceptance object!"
                << std::endl;
    }
  }

  if (model_opt_ptree.get<bool>(
          "resolution_smearing_active")) { // with resolution smearing
    // ok since we have smearing on, generate smearing model
  }

  // every model has superior parameters which have to be set by the user
  // if they are kept fixed
  // in this case its the lab momentum plab
  current_model->getModelParameterSet().setModelParameterValue(
      "p_lab", data.getLabMomentum());
  // the luminosity (we set it to 1.0 as a default value)
  current_model->getModelParameterSet().setModelParameterValue("luminosity",
                                                               1.0);
  if (current_model->init()) {
    std::cout << "ERROR: Not all parameters of the model were successfully "
                 "initialized!"
              << std::endl;
    current_model->getModelParameterSet().printInfo();
  }

  return current_model;
}

std::shared_ptr<Model2D> PndLmdModelFactory::generate2DModel(
    const boost::property_tree::ptree &model_opt_ptree,
    const PndLmdAngularData &data) const {
  std::shared_ptr<Model2D> current_model;

  std::stringstream model_name;
  model_name << "dpm_angular_2d";

  LumiFit::TransformationOption trans_option =
      LumiFit::StringToTransformationOption.at(
          model_opt_ptree.get<std::string>("theta_t_trafo_option"));

  std::shared_ptr<PndLmdDPMAngModel1D> dpm_angular_1d(new PndLmdDPMAngModel1D(
      "dpm_angular_1d",
      LumiFit::StringToDPMElasticParts.at(
          model_opt_ptree.get<std::string>("dpm_elastic_parts")),
      trans_option));

  std::shared_ptr<Parametrization> dpm_parametrization(
      new PndLmdDPMModelParametrization(
          dpm_angular_1d->getModelParameterSet()));
  dpm_angular_1d->getModelParameterHandler().registerParametrizations(
      dpm_angular_1d->getModelParameterSet(), dpm_parametrization);

  const LumiFit::LmdDimension &data_primary_dimension =
      data.getPrimaryDimension();
  const LumiFit::LmdDimension &data_secondary_dimension =
      data.getSecondaryDimension();

  if (data_primary_dimension.dimension_options.dimension_type ==
          LumiFit::THETA_X &&
      data_secondary_dimension.dimension_options.dimension_type ==
          LumiFit::THETA_Y) {

    std::cout << "using fast 2d dpm model..." << std::endl;

    LumiFit::LmdDimension temp_prim_dim(data.getPrimaryDimension());
    LumiFit::LmdDimension temp_sec_dim(data.getSecondaryDimension());
    unsigned int combine = 1;
    temp_prim_dim.bins *= combine;
    temp_sec_dim.bins *= combine;
    temp_prim_dim.calculateBinSize();
    temp_sec_dim.calculateBinSize();

    std::shared_ptr<PndLmdFastDPMAngModel2D> dpm_model_2d(
        new PndLmdFastDPMAngModel2D(model_name.str(), dpm_angular_1d));

    model_name << "_cached";

    current_model.reset(new CachedModel2D(model_name.str(), dpm_model_2d,
                                          temp_prim_dim, temp_sec_dim));

    if (model_opt_ptree.get<bool>("divergence_smearing_active") &&
        data_primary_dimension.is_active &&
        data_secondary_dimension.is_active) {
      // add the divergence smearing on top
      // at this point we introduce "non numerical" discretization from the data
      // thats why the data object is required here

      std::shared_ptr<GaussianModel2D> divergence_model(
          new GaussianModel2D("divergence_model", 5.0));

      divergence_model->init();

      std::shared_ptr<PndLmdDivergenceSmearingModel2D>
          divergence_smearing_model(new PndLmdDivergenceSmearingModel2D(
              divergence_model, temp_prim_dim, temp_sec_dim));

      model_name << "_div_smeared";

      std::shared_ptr<PndLmdDifferentialSmearingConvolutionModel2D>
          div_smeared_model(new PndLmdDifferentialSmearingConvolutionModel2D(
              model_name.str(), current_model, divergence_smearing_model,
              data_primary_dimension, data_secondary_dimension, combine));
      div_smeared_model->injectModelParameter(
          divergence_model->getModelParameterSet().getModelParameter(
              "gauss_sigma_var1"));
      div_smeared_model->injectModelParameter(
          divergence_model->getModelParameterSet().getModelParameter(
              "gauss_sigma_var2"));

      current_model = div_smeared_model;
    }
  } else {
    if (model_opt_ptree.get<bool>("divergence_smearing_active") &&
        data_primary_dimension.is_active &&
        data_secondary_dimension.is_active) {
      std::cout << "WARNING: Divergence smearing is not available in this "
                   "coordinate system!<<"
                   " Please use the theta_xy frame."
                << std::endl;
    }

    std::cout << "using correct 2d dpm model..." << std::endl;
    current_model.reset(
        new PndLmdDPMAngModel2D(model_name.str(), dpm_angular_1d));
  }

  if (model_opt_ptree.get<bool>(
          "acceptance_correction_active")) { // with acceptance corr
    if (acceptance.getAcceptance2D()) {
      std::shared_ptr<DataModel2D> acc(new DataModel2D(
          "acceptance_2d",
          LumiFit::StringToInterpolationType.at(
              model_opt_ptree.get<std::string>("acceptance_interpolation"))));

      TVirtualPad *curpad = gPad;
      TCanvas c;
      TEfficiency *eff2 = acceptance.getAcceptance2D();
      eff2->Draw("colz");
      c.Update();
      TH2 *hist = acceptance.getAcceptance2D()->GetPaintedHistogram();

      double angular_offsets[2];
      angular_offsets[0] = 0.0;
      angular_offsets[1] = 0.0;

      if (model_opt_ptree.get<bool>("automatic_acceptance_shifting_active")) {
        std::cout << "Applying automatic acceptance shifting!\n";
        // get offset values as measured by the offset determination (user
        // should have specified that)
        std::pair<double, double> ip_offsets = data.getIPOffsets();
        // then read transformation json file
        auto pt = PndLmdRuntimeConfiguration::Instance()
                      .getAcceptanceOffsetsTranformationParameters();

        double trans_params[6];
        trans_params[0] = pt.get_child("matrix").get<double>("m11");
        trans_params[1] = pt.get_child("matrix").get<double>("m12");
        trans_params[2] = pt.get_child("matrix").get<double>("m21");
        trans_params[3] = pt.get_child("matrix").get<double>("m22");
        trans_params[4] = pt.get_child("before_translation").get<double>("t1");
        trans_params[5] = pt.get_child("before_translation").get<double>("t2");

        // and do the transformation from offset coordinates to the angular
        // shift coordinates

        ip_offsets.first += trans_params[4];
        ip_offsets.second += trans_params[5];
        angular_offsets[0] = trans_params[0] * ip_offsets.first +
                             trans_params[1] * ip_offsets.second;
        angular_offsets[1] = trans_params[2] * ip_offsets.first +
                             trans_params[3] * ip_offsets.second;
        angular_offsets[0] /= 1000.0; // convert to mrad
        angular_offsets[1] /= 1000.0;
      }

      std::pair<mydouble, mydouble> acceptance_masscenter(0.0, 0.0);
      double acceptancesum(0.0);

      // then use these coordinates to create a corrected acceptance
      std::pair<mydouble, mydouble> pos;
      std::pair<mydouble, mydouble> eval_pos;
      std::map<std::pair<mydouble, mydouble>, mydouble> datamap;
      for (unsigned int ix = 0; ix < data_primary_dimension.bins; ix++) {
        for (unsigned int iy = 0; iy < data_secondary_dimension.bins; iy++) {
          pos.first = data_primary_dimension.dimension_range.getRangeLow() +
                      (0.5 + ix) * data_primary_dimension.bin_size;
          pos.second = data_secondary_dimension.dimension_range.getRangeLow() +
                       (0.5 + iy) * data_secondary_dimension.bin_size;

          eval_pos.first = pos.first - angular_offsets[0];
          eval_pos.second = pos.second - angular_offsets[1];

          if (eval_pos.first <
                  data_primary_dimension.dimension_range.getRangeLow() ||
              eval_pos.first >
                  data_primary_dimension.dimension_range.getRangeHigh() ||
              eval_pos.second <
                  data_secondary_dimension.dimension_range.getRangeLow() ||
              eval_pos.first >
                  data_secondary_dimension.dimension_range.getRangeHigh()) {
            datamap[pos] = 0.0;
          } else {
            datamap[pos] = hist->Interpolate(eval_pos.first, eval_pos.second);
          }

          acceptance_masscenter.first += datamap[pos] * eval_pos.first;
          acceptance_masscenter.second += datamap[pos] * eval_pos.second;
          acceptancesum += datamap[pos];
        }
      }
      acceptance_masscenter.first /= acceptancesum;
      acceptance_masscenter.second /= acceptancesum;

      boost::optional<bool> clean_lower_acceptance =
          model_opt_ptree.get_optional<bool>("clean_lower_acceptance");
      if (clean_lower_acceptance.is_initialized() &&
          clean_lower_acceptance.get()) {
        // clean the acceptance! Non-zero efficiency values inside the
        // acceptance hole can lead to systematic errors due to the divergence
        // of the model in this domain. These efficiency values have to be set
        // to zero!
        mydouble inner_acceptance_edge_inner_radius(0.002);
        mydouble inner_acceptance_edge_upper_radius_squared(std::pow(0.004, 2));

        boost::optional<double> forced_lower_acc_bound =
            model_opt_ptree.get_optional<double>(
                "override_lower_acceptance_bound");
        if (forced_lower_acc_bound) {
          inner_acceptance_edge_inner_radius = forced_lower_acc_bound.get();
          std::cout << "INFO: using forced lower acceptance radius of "
                    << forced_lower_acc_bound.get() << std::endl;
        }

        // clean inner part of acceptance
        double efficiency_threshold(0.2);
        std::cout << "INFO: setting acceptance to zero in the circle around ("
                  << acceptance_masscenter.first << ","
                  << acceptance_masscenter.second << ") with a radius of "
                  << inner_acceptance_edge_inner_radius << std::endl;
        double lowboundsquared(std::pow(inner_acceptance_edge_inner_radius, 2));
        std::vector<std::pair<mydouble, mydouble>> keystoremove;
        for (auto const &x : datamap) {
          double distance_from_center(
              std::pow(x.first.first - acceptance_masscenter.first, 2) +
              std::pow(x.first.second - acceptance_masscenter.second, 2));
          if (distance_from_center < lowboundsquared) {
            // just remove efficiency values inside the
            keystoremove.push_back(x.first);
          } else if (distance_from_center <
                     inner_acceptance_edge_upper_radius_squared) {
            if (x.second < efficiency_threshold) {
              // we only cut acceptance edges below threshold
              // this cleans up the inner acceptance edge
              keystoremove.push_back(x.first);
            }
          }
        }

        std::cout << "INFO: erasing " << keystoremove.size()
                  << " efficiency entries, which are either:"
                  << "\n- below the bound of "
                  << inner_acceptance_edge_inner_radius
                  << "\n- below the bound of "
                  << std::sqrt(inner_acceptance_edge_upper_radius_squared)
                  << " and a efficiency below " << efficiency_threshold
                  << std::endl;
        for (auto k : keystoremove)
          datamap.erase(k);
      }

      gPad = curpad;

      acc->setData(datamap);

      model_name << "_acceptance_corrected";

      current_model.reset(
          new ProductModel2D(model_name.str(), current_model, acc));

      current_model->getModelParameterSet()
          .getModelParameter("offset_x")
          ->setValue(0.0);
      current_model->getModelParameterSet()
          .getModelParameter("offset_y")
          ->setValue(0.0);
    } else {
      std::cout << "ERROR: requesting an acceptance corrected model without an "
                   "acceptance object!"
                << std::endl;
    }
  }

  if (model_opt_ptree.get<bool>(
          "resolution_smearing_active")) { // with resolution smearing
    // ok since we have smearing on, generate smearing model

    model_name << "_res_smeared";

    current_model.reset(new PndLmdSmearingConvolutionModel2D(
        model_name.str(), current_model,
        generate2DSmearingModel(data.getPrimaryDimension(),
                                data.getSecondaryDimension()),
        data.getPrimaryDimension(), data.getSecondaryDimension()));
  }

  // every model has superior parameters which have to be set by the user
  // if they are kept fixed
  // in this case its the lab momentum plab
  dpm_angular_1d->getModelParameterSet().setModelParameterValue(
      "p_lab", data.getLabMomentum());
  // free the luminosity (we set it to 1.0 as a default value)
  /*dpm_angular_1d->getModelParameterSet().freeModelParameter("luminosity");
   dpm_angular_1d->getModelParameterSet().setModelParameterValue("luminosity",
   1.0);*/

  current_model->init();

  return current_model;
}

const PndLmdAcceptance &PndLmdModelFactory::getAcceptance() const {
  return acceptance;
}
const PndLmdMapData &PndLmdModelFactory::getResolutionMapData() const {
  return resolution_map_data;
}

void PndLmdModelFactory::setAcceptance(const PndLmdAcceptance &acceptance_) {
  acceptance = acceptance_;
}

void PndLmdModelFactory::setResolutionMapData(const PndLmdMapData &res_map_) {
  resolution_map_data = res_map_;
}

void PndLmdModelFactory::setResolutions(
    const std::vector<PndLmdHistogramData> &resolutions_) {
  resolutions = resolutions_;
}

std::shared_ptr<Model1D> PndLmdModelFactory::generate1DVertexModel(
    const boost::property_tree::ptree &model_opt_ptree) const {
  std::shared_ptr<Model1D> vertex_model;

  LumiFit::ModelType model_type(LumiFit::StringToModelType.at(
      model_opt_ptree.get<std::string>("model_type")));

  if (model_type == LumiFit::UNIFORM) {
    vertex_model.reset(new DoubleGaussianModel1D("vertex_double_gaussian_1d"));
  } else if (model_type == LumiFit::GAUSSIAN) {
    vertex_model.reset(new GaussianModel1D("vertex_gaussian_1d"));
  } else {
    std::cout
        << "Unknown vertex model! Only uniform and gaussian model is allowed."
        << std::endl;
  }

  return vertex_model;
}

std::shared_ptr<Model> PndLmdModelFactory::generateModel(
    const boost::property_tree::ptree &model_opt_ptree,
    const PndLmdAngularData &data) const {
  std::shared_ptr<Model> current_model;

  std::cout << "Generating model ... " << std::endl;

  unsigned int fit_dimension =
      model_opt_ptree.get<unsigned int>("fit_dimension");
  if (fit_dimension == 1) {
    current_model = generate1DModel(model_opt_ptree, data);
  } else if (fit_dimension == 2) {
    current_model = generate2DModel(model_opt_ptree, data);
  } else {
    std::cout << "WARNING: Requesting a model of dimension " << fit_dimension
              << " which is NOT available! Returning NULL pointer."
              << std::endl;
  }
  return current_model;
}
