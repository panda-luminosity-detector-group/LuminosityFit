/*
 * DoubleGaussianModel1D.cxx
 *
 *  Created on: Feb 19, 2013
 *      Author: steve
 */

#include "DoubleGaussianModel1D.h"

#define _USE_MATH_DEFINES
#include "math.h"
#include <iostream>
#include <limits>
DoubleGaussianModel1D::DoubleGaussianModel1D(std::string name_)
    : Model1D(name_) {
  num_sigmas = 5.0;
  initModelParameters();
}

DoubleGaussianModel1D::~DoubleGaussianModel1D() {
  // TODO Auto-generated destructor stub
}
/*
void DoubleGaussianModel1D::initModelParameters() {
        gauss_mean_narrow = getModelParameterSet().addModelParameter(
                        "gauss_mean_narrow");
        gauss_mean_wide =
getModelParameterSet().addModelParameter("gauss_mean_wide"); gauss_sigma_narrow
= getModelParameterSet().addModelParameter( "gauss_sigma_narrow");
        gauss_sigma_wide = getModelParameterSet().addModelParameter(
                        "gauss_sigma_wide");
        gauss_ratio_narrow_wide = getModelParameterSet().addModelParameter(
                        "gauss_ratio_narrow_wide");
        gauss_ratio_narrow_wide->setParameterBounds(0.0,
                        std::numeric_limits<double>::max());
        // as default the amplitude of the gauss is set to 1,
        // which makes it a normal distribution
        gauss_amplitude =
getModelParameterSet().addModelParameter("gauss_amplitude");
        gauss_amplitude->setValue(1.0);
        gauss_amplitude->setParameterFixed(true);
}

double DoubleGaussianModel1D::eval(const double *x) const {
        // double gauss smearing function
        return gauss_amplitude->getValue() * gauss_ratio_narrow_wide->getValue()
                        / (gauss_ratio_narrow_wide->getValue() + 1.0)
                        / (gauss_sigma_narrow->getValue() * sqrt(2.0 * M_PI))
                        * exp(
                                        -(pow(x[0] -
gauss_mean_narrow->getValue(), 2.0) / (2.0 * gauss_sigma_narrow->getValue()
                                                                        * gauss_sigma_narrow->getValue())))
                        + gauss_amplitude->getValue()
                                        / (gauss_ratio_narrow_wide->getValue()
+ 1.0) / (gauss_sigma_wide->getValue() * sqrt(2.0 * M_PI))
                                        * exp(
                                                        -(pow(x[0] -
gauss_mean_wide->getValue(), 2.0) / (2.0 *
pow(gauss_sigma_wide->getValue(), 2.0))));
}

void DoubleGaussianModel1D::updateDomain() {
        double temp = num_sigmas * gauss_sigma_wide->getValue();
        setDomain(-temp + gauss_mean_wide->getValue(),
                        temp + gauss_mean_wide->getValue());
}*/

void DoubleGaussianModel1D::initModelParameters() {
  gauss_mean_narrow =
      getModelParameterSet().addModelParameter("gauss_mean_narrow");
  gauss_mean_wide = getModelParameterSet().addModelParameter("gauss_mean_wide");
  gauss_sigma_narrow =
      getModelParameterSet().addModelParameter("gauss_sigma_narrow");
  gauss_sigma_ratio_narrow_wide =
      getModelParameterSet().addModelParameter("gauss_sigma_ratio_narrow_wide");
  gauss_ratio_narrow_wide =
      getModelParameterSet().addModelParameter("gauss_ratio_narrow_wide");
  gauss_ratio_narrow_wide->setParameterBounds(
      0.0, std::numeric_limits<double>::max());
  // as default the amplitude of the gauss is set to 1,
  // which makes it a normal distribution
  gauss_amplitude = getModelParameterSet().addModelParameter("gauss_amplitude");
  gauss_amplitude->setValue(1.0);
  gauss_amplitude->setParameterFixed(true);
}

mydouble DoubleGaussianModel1D::eval(const mydouble *x) const {
  // double gauss smearing function
  return gauss_amplitude->getValue() * gauss_ratio_narrow_wide->getValue() /
             (gauss_ratio_narrow_wide->getValue() + 1.0) /
             (gauss_sigma_narrow->getValue() * sqrt(2.0 * M_PI)) *
             exp(-(pow(x[0] - gauss_mean_narrow->getValue(), 2.0) /
                   (2.0 * gauss_sigma_narrow->getValue() *
                    gauss_sigma_narrow->getValue()))) +
         gauss_amplitude->getValue() /
             (gauss_ratio_narrow_wide->getValue() + 1.0) /
             (gauss_sigma_narrow->getValue() /
              gauss_sigma_ratio_narrow_wide->getValue() * sqrt(2.0 * M_PI)) *
             exp(-(pow(x[0] - gauss_mean_wide->getValue(), 2.0) /
                   (2.0 * pow(gauss_sigma_narrow->getValue() /
                                  gauss_sigma_ratio_narrow_wide->getValue(),
                              2.0))));
}

void DoubleGaussianModel1D::updateDomain() {
  mydouble temp = num_sigmas * gauss_sigma_narrow->getValue() /
                  gauss_sigma_ratio_narrow_wide->getValue();
  setDomain(-temp + gauss_mean_wide->getValue(),
            temp + gauss_mean_wide->getValue());
}
