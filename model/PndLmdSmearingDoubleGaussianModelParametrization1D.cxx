/*
 * PndLmdSmearingDoubleGaussianModelParametrization1D.cxx
 *
 *  Created on: Mar 28, 2013
 *      Author: steve
 */

#include "PndLmdSmearingDoubleGaussianModelParametrization1D.h"
#include "core/Model1D.h"
#include "core/ModelPar.h"
#include "models1d/ExponentialModel1D.h"
#include "models1d/PolynomialModel1D.h"
#include "operators1d/AdditionModel1D.h"

PndLmdSmearingDoubleGaussianModelParametrization1D::
    PndLmdSmearingDoubleGaussianModelParametrization1D(
        std::shared_ptr<Model1D> model) {

  std::shared_ptr<Model1D> poly_model1(
      new PolynomialModel1D("gauss_mean_narrow_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm1(
      new ParametrizationModel(poly_model1));
  model->getModelParameterHandler().registerParametrizationModel(
      model->getModelParameterSet().getModelParameter("gauss_mean_narrow"),
      pm1);

  std::shared_ptr<Model1D> poly_model2(
      new PolynomialModel1D("gauss_mean_wide_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm2(
      new ParametrizationModel(poly_model2));
  model->getModelParameterHandler().registerParametrizationModel(
      model->getModelParameterSet().getModelParameter("gauss_mean_wide"), pm2);

  std::shared_ptr<Model1D> poly_model3(
      new PolynomialModel1D("gauss_sigma_narrow_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm3(
      new ParametrizationModel(poly_model3));
  model->getModelParameterHandler().registerParametrizationModel(
      model->getModelParameterSet().getModelParameter("gauss_sigma_narrow"),
      pm3);

  std::shared_ptr<Model1D> poly_model4(
      new PolynomialModel1D("gauss_sigma_ratio_narrow_wide_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm4(
      new ParametrizationModel(poly_model4));
  model->getModelParameterHandler().registerParametrizationModel(
      model->getModelParameterSet().getModelParameter(
          "gauss_sigma_ratio_narrow_wide"),
      pm4);

  std::shared_ptr<Model1D> poly_model5(
      new PolynomialModel1D("gauss_ratio_narrow_wide_poly_model_1d", 1));
  std::shared_ptr<ParametrizationModel> pm5(
      new ParametrizationModel(poly_model5));
  model->getModelParameterHandler().registerParametrizationModel(
      model->getModelParameterSet().getModelParameter(
          "gauss_ratio_narrow_wide"),
      pm5);

  /*p_lab = model->getModelParameterSet().addModelParameter("p_lab");
  p_lab->setSuperior(true);

  std::shared_ptr<Model1D> poly_model1(
                  new PolynomialModel1D("gauss_sigma_narrow_poly_model_1d", 0));
  poly_model1->getModelParameterSet().setModelParameterValue(
                  "poly_poly_factor_0", 0.03449);
  std::shared_ptr<Model1D> exp_model1(
                  new ExponentialModel1D("gauss_sigma_narrow_exp_model_1d"));
  exp_model1->getModelParameterSet().setModelParameterValue("exp_amplitude",
                  0.4413);
  exp_model1->getModelParameterSet().setModelParameterValue("exp_exp_factor",
                  -0.5409);
  std::shared_ptr<Model1D> add_model1(
                  new
  AdditionModel1D("gauss_sigma_narrow_poly_model_plus_exp_model_1d", exp_model1,
                                  poly_model1));
  std::shared_ptr<ParametrizationModel> pm1(new
  ParametrizationModel(add_model1));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_sigma_narrow"),
  pm1);

  std::shared_ptr<Model1D> poly_model2(
                  new PolynomialModel1D("gauss_sigma_wide_poly_model_1d", 1));
  poly_model2->getModelParameterSet().setModelParameterValue(
                  "poly_poly_factor_0", 1.044);
  poly_model2->getModelParameterSet().setModelParameterValue(
                  "poly_poly_factor_1", -0.007297);
  std::shared_ptr<ParametrizationModel> pm2(new
  ParametrizationModel(poly_model2));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_sigma_wide"),
  pm2);

  std::shared_ptr<Model1D> poly_model3(
                  new PolynomialModel1D("gauss_ratio_narrow_wide_poly_model_1d",
  0)); poly_model3->getModelParameterSet().setModelParameterValue(
                  "poly_poly_factor_0", 0.3045);
  std::shared_ptr<ParametrizationModel> pm3(new
  ParametrizationModel(poly_model3));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_ratio_narrow_wide"),
  pm3);

  std::shared_ptr<Model1D> poly_model4(
                  new PolynomialModel1D("gauss_mean_narrow_poly_model_1d", 0));
  poly_model4->getModelParameterSet().setModelParameterValue(
                  "poly_poly_factor_0", 0.03449);
  std::shared_ptr<Model1D> exp_model2(
                  new ExponentialModel1D("gauss_mean_narrow_exp_model_1d"));
  exp_model2->getModelParameterSet().setModelParameterValue("exp_amplitude",
                  0.4413);
  exp_model2->getModelParameterSet().setModelParameterValue("exp_exp_factor",
                  -0.5409);
  std::shared_ptr<Model1D> add_model2(
                  new
  AdditionModel1D("gauss_mean_narrow_poly_model_plus_exp_model_1d", exp_model2,
                                  poly_model4));
  std::shared_ptr<ParametrizationModel> pm4(new
  ParametrizationModel(add_model2));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_mean_narrow"),
  pm4);

  std::shared_ptr<Model1D> poly_model5(
                  new PolynomialModel1D("gauss_mean_wide_poly_model_1d", 0));
  poly_model5->getModelParameterSet().setModelParameterValue(
                  "poly_poly_factor_0", 0.03449);
  std::shared_ptr<Model1D> exp_model3(
                  new ExponentialModel1D("gauss_mean_wide_exp_model_1d"));
  exp_model3->getModelParameterSet().setModelParameterValue("exp_amplitude",
                  0.4413);
  exp_model3->getModelParameterSet().setModelParameterValue("exp_exp_factor",
                  -0.5409);
  std::shared_ptr<Model1D> add_model3(
                  new
  AdditionModel1D("gauss_mean_wide_poly_model_plus_exp_model_1d", exp_model3,
                                  poly_model5));
  std::shared_ptr<ParametrizationModel> pm5(new
  ParametrizationModel(add_model3));
  model->getModelParameterHandler().registerParametrizationModel(
                  model->getModelParameterSet().getModelParameter("gauss_mean_wide"),
  pm5);*/
}

PndLmdSmearingDoubleGaussianModelParametrization1D::
    ~PndLmdSmearingDoubleGaussianModelParametrization1D() {
  
}
