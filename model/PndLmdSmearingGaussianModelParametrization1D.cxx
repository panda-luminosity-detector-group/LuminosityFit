/*
 * PndLmdSmearingGaussianModelParametrization1D.cxx
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#include "PndLmdSmearingGaussianModelParametrization1D.h"
#include "LumiFitStructs.h"
#include "core/Model1D.h"
#include "core/ModelPar.h"
#include "model/PndLmdROOTDataModel1D.h"
#include "models1d/PolynomialModel1D.h"

#include <string>

#include "TFile.h"
#include "TGraphAsymmErrors.h"

PndLmdSmearingGaussianModelParametrization1D::
    PndLmdSmearingGaussianModelParametrization1D(
        std::shared_ptr<Model1D> model,
        const LumiFit::PndLmdFitModelOptions &model_options) {
  p_lab = model->getModelParameterSet().addModelParameter("p_lab");
  p_lab->setSuperior(true);

  /*if (!model_options.use_resolution_parameter_interpolation) {
  std::shared_ptr<Model1D> poly_model1(
                  new PolynomialModel1D("gauss_sigma_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm1(new
ParametrizationModel(poly_model1));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_sigma"),
                  pm1);

  std::shared_ptr<Model1D> poly_model2(
                  new PolynomialModel1D("gauss_mean_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm2(new
ParametrizationModel(poly_model2));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_mean"),
                  pm2);

//
model->getModelParameterSet().getModelParameter("gauss_mean")->setValue(0.0); }
else { std::shared_ptr<PndLmdROOTDataModel1D> interpolation_model1( new
PndLmdROOTDataModel1D("gauss_sigma_data_model_1d"));
          std::shared_ptr<ParametrizationModel> pm1(
                          new ParametrizationModel(interpolation_model1));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter(
                                          "gauss_sigma"), pm1);

          std::shared_ptr<PndLmdROOTDataModel1D> interpolation_model2(
                          new
PndLmdROOTDataModel1D("gauss_mean_data_model_1d"));
          std::shared_ptr<ParametrizationModel> pm2(
                          new ParametrizationModel(interpolation_model2));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter("gauss_mean"),
                          pm2);

          if (model_options.resolution_smearing_active) {
                  PndLmdLumiHelper lmd_helper;

                  TFile
f(model_options.resolution_parametrization_file_url.c_str(), "READ");

                  std::vector<PndLmdLumiHelper::lmd_graph*> allgraphs =
                                  lmd_helper.getResolutionModelResultsFromFile(&f);
                  std::set<LumiFit::LmdDimensionType> dependencies;
                  dependencies.insert(LumiFit::THETA);
                  std::vector<PndLmdLumiHelper::lmd_graph*> graphs =
                                  lmd_helper.filterLmdGraphs(allgraphs,
dependencies); graphs = lmd_helper.filterLmdGraphs(graphs,
model_options.selections);

                  for (unsigned int i = 0; i < graphs.size(); i++) {
                          std::map<unsigned int, std::pair<std::string,
std::string> >::const_iterator parameter_name =
                                          graphs[i]->parameter_name_stack.begin();
                          std::cout << parameter_name->second.first << " "
                                          << parameter_name->second.second <<
std::endl; if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_sigma"))) {
                                  TGraphAsymmErrors *gr = new TGraphAsymmErrors(
                                                  graphs[i]->graph->GetN(),
graphs[i]->graph->GetX(), graphs[i]->graph->GetY(),
graphs[i]->graph->GetEXlow(), graphs[i]->graph->GetEXhigh(),
graphs[i]->graph->GetEYlow(), graphs[i]->graph->GetEYhigh());
                                  interpolation_model1->setGraph(gr);
                                  interpolation_model1->setIntpolType(LumiFit::LINEAR);

                          } else if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_mean"))) {
                                  TGraphAsymmErrors *gr = new TGraphAsymmErrors(
                                                  graphs[i]->graph->GetN(),
graphs[i]->graph->GetX(), graphs[i]->graph->GetY(),
graphs[i]->graph->GetEXlow(), graphs[i]->graph->GetEXhigh(),
graphs[i]->graph->GetEYlow(), graphs[i]->graph->GetEYhigh());
                                  interpolation_model2->setGraph(gr);
                                  interpolation_model2->setIntpolType(LumiFit::LINEAR);

                          } else {
                                  std::cout
                                                  << "ERROR: Not able to obtain
parametrization model for parameter "
                                                  <<
parameter_name->second.second << "!" << std::endl;
                          }
                  }
          }
  }*/
}

PndLmdSmearingGaussianModelParametrization1D::
    ~PndLmdSmearingGaussianModelParametrization1D() {
  // TODO Auto-generated destructor stub
}
