/*
 * PndLmdSmearingGaussianModelParametrization2D.cxx
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#include "PndLmdSmearingGaussianModelParametrization2D.h"
#include "LumiFitStructs.h"
#include "core/Model2D.h"
#include "core/ModelPar.h"
#include "model/PndLmdROOTDataModel2D.h"
#include "models1d/PolynomialModel1D.h"

#include <string>

#include "TFile.h"
#include "TGraphAsymmErrors.h"

PndLmdSmearingGaussianModelParametrization2D::
    PndLmdSmearingGaussianModelParametrization2D(
        std::shared_ptr<Model2D> model,
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
else { std::shared_ptr<PndLmdROOTDataModel2D> interpolation_model_sigma1( new
PndLmdROOTDataModel2D("gauss_sigma_var1_data_model_2d"));
          std::shared_ptr<ParametrizationModel> pm_sigma1(
                          new ParametrizationModel(interpolation_model_sigma1));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter("gauss_sigma_var1"),
                          pm_sigma1);
          std::shared_ptr<PndLmdROOTDataModel2D> interpolation_model_sigma2(
                          new
PndLmdROOTDataModel2D("gauss_sigma_var2_data_model_2d"));
          std::shared_ptr<ParametrizationModel> pm_sigma2(
                          new ParametrizationModel(interpolation_model_sigma2));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter("gauss_sigma_var2"),
                          pm_sigma2);

          std::shared_ptr<PndLmdROOTDataModel2D> interpolation_model_mean1(
                          new
PndLmdROOTDataModel2D("gauss_mean_var1_data_model_2d"));
          std::shared_ptr<ParametrizationModel> pm_mean1(
                          new ParametrizationModel(interpolation_model_mean1));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter("gauss_mean_var1"),
                          pm_mean1);
          std::shared_ptr<PndLmdROOTDataModel2D> interpolation_model_mean2(
                          new
PndLmdROOTDataModel2D("gauss_mean_var2_data_model_2d"));
          std::shared_ptr<ParametrizationModel> pm_mean2(
                          new ParametrizationModel(interpolation_model_mean2));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter("gauss_mean_var2"),
                          pm_mean2);

          std::shared_ptr<PndLmdROOTDataModel2D> interpolation_model_rho(
                          new PndLmdROOTDataModel2D("gauss_rho_data_model_2d"));
          std::shared_ptr<ParametrizationModel> pm_rho(
                          new ParametrizationModel(interpolation_model_rho));
          model->getModelParameterHandler().registerParametrizationModel(
                          model->getModelParameterSet().getModelParameter("gauss_rho"),
pm_rho);

          if (model_options.resolution_smearing_active) {
                  PndLmdLumiHelper lmd_helper;

                  TFile
f(model_options.resolution_parametrization_file_url.c_str(), "READ");

                  std::vector<PndLmdLumiHelper::lmd_graph*> allgraphs =
                                  lmd_helper.getResolutionModelResultsFromFile(&f);

                  std::set<LumiFit::LmdDimensionType> dependencies;
                  dependencies.insert(LumiFit::THETA);
                  dependencies.insert(LumiFit::PHI);

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
std::endl;\ TGraph2DErrors *graph2d = graphs[i]->graph2d; TGraph2DErrors *gr(0);
                          if (graph2d)
                                  gr = new TGraph2DErrors(graph2d->GetN(),
graph2d->GetX(), graph2d->GetY(), graph2d->GetZ(), graph2d->GetEX(),
                                                  graph2d->GetEY(),
graph2d->GetEZ()); if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_sigma_var1"))) {
                                  interpolation_model_sigma1->setGraph(gr);
                                  interpolation_model_sigma1->setIntpolType(LumiFit::CONSTANT);
                          } else if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_sigma_var2"))) {
                                  interpolation_model_sigma2->setGraph(gr);
                                  interpolation_model_sigma2->setIntpolType(LumiFit::CONSTANT);
                          } else if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_mean_var1"))) {
                                  interpolation_model_mean1->setGraph(gr);
                                  interpolation_model_mean1->setIntpolType(LumiFit::CONSTANT);
                          } else if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_mean_var2"))) {
                                  interpolation_model_mean2->setGraph(gr);
                                  interpolation_model_mean2->setIntpolType(LumiFit::CONSTANT);
                          } else if (0
                                          ==
parameter_name->second.second.compare( std::string("gauss_rho"))) {
                                  interpolation_model_rho->setGraph(gr);
                                  interpolation_model_rho->setIntpolType(LumiFit::CONSTANT);
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

PndLmdSmearingGaussianModelParametrization2D::
    ~PndLmdSmearingGaussianModelParametrization2D() {
  // TODO Auto-generated destructor stub
}
