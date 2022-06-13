/*
 * GaussianModel2D.cxx
 *
 *  Created on: Jan 16, 2013
 *      Author: steve
 */

#include "GaussianModel2D.h"

#include <cmath>

GaussianModel2D::GaussianModel2D(std::string name_, mydouble num_sigmas_)
    : Model2D(name_), num_sigmas(num_sigmas_), gauss_sigma_var1(),
      gauss_sigma_var2(), gauss_mean_var1(), gauss_mean_var2(), gauss_rho(),
      gauss_amplitude() {
  initModelParameters();
}

GaussianModel2D::~GaussianModel2D() {
  
}

void GaussianModel2D::initModelParameters() {
  gauss_sigma_var1 =
      getModelParameterSet().addModelParameter("gauss_sigma_var1");
  gauss_sigma_var2 =
      getModelParameterSet().addModelParameter("gauss_sigma_var2");
  gauss_mean_var1 = getModelParameterSet().addModelParameter("gauss_mean_var1");
  gauss_mean_var2 = getModelParameterSet().addModelParameter("gauss_mean_var2");
  gauss_rho = getModelParameterSet().addModelParameter("gauss_rho");
  // as default the amplitude of the gauss is set to 1,
  // which makes it a normal distribution
  gauss_amplitude = getModelParameterSet().addModelParameter("gauss_amplitude");
  gauss_amplitude->setValue(1.0);
  gauss_amplitude->setParameterFixed(true);
}

mydouble GaussianModel2D::eval(const mydouble *x) const {
  // see wikipedia definition

  mydouble rho_factor = (1.0 - std::pow(gauss_rho->getValue(), 2));
  mydouble one_over_sigma_var1 = 1.0 / gauss_sigma_var1->getValue();
  mydouble one_over_sigma_var2 = 1.0 / gauss_sigma_var2->getValue();

  mydouble xval = one_over_sigma_var1 * (x[0] - gauss_mean_var1->getValue());
  mydouble yval = one_over_sigma_var2 * (x[1] - gauss_mean_var2->getValue());

  mydouble normalization = 0.5 * one_over_sigma_var1 * one_over_sigma_var2 /
                           (M_PI * sqrt(rho_factor));

  mydouble exp_value = exp(
      -0.5 / rho_factor *
      (xval * xval + yval * yval - 2.0 * gauss_rho->getValue() * xval * yval));

  return normalization * exp_value * gauss_amplitude->getValue();
}

void GaussianModel2D::updateDomain() {
  mydouble temp = num_sigmas * std::abs(gauss_sigma_var1->getValue());
  setVar1Domain(-temp + gauss_mean_var1->getValue(),
                temp + gauss_mean_var1->getValue());
  temp = num_sigmas * std::abs(gauss_sigma_var2->getValue());
  setVar2Domain(-temp + gauss_mean_var2->getValue(),
                temp + gauss_mean_var2->getValue());
}
