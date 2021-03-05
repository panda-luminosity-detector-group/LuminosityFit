#include "model/PndLmdModelFactory.h"
#include <iostream>

#include "boost/property_tree/ptree.hpp"

#include "TMath.h"

int main(int argc, char *argv[]) {
  PndLmdAngularData data;
  data.setLabMomentum(1.5);

  // create 2d model and initialize tilt parameters
  PndLmdModelFactory model_factory;

  boost::property_tree::ptree model_opt;
  model_opt.put("fit_dimension", 2);
  model_opt.put("acceptance_correction_active", false);
  model_opt.put("resolution_smearing_active", false);

  LumiFit::LmdDimension lmd_dim;
  data.setPrimaryDimension(lmd_dim);

  shared_ptr<Model> model_old = model_factory.generateModel(model_opt, data);
  if (model_old->init()) {
    std::cout << "Error: not all parameters have been set!" << std::endl;
  }

  model_old->getModelParameterSet().freeModelParameter("tilt_x");
  model_old->getModelParameterSet().freeModelParameter("tilt_y");

  lmd_dim.dimension_options.dimension_type = LumiFit::THETA_X;
  data.setPrimaryDimension(lmd_dim);

  shared_ptr<Model> model_fast = model_factory.generateModel(model_opt, data);
  if (model_fast->init()) {
    std::cout << "Error: not all parameters have been set!" << std::endl;
  }

  model_fast->getModelParameterSet().freeModelParameter("tilt_x");
  model_fast->getModelParameterSet().freeModelParameter("tilt_y");

  double tilt_x(0.0);
  double tilt_y(0.0);
  model_old->getModelParameterSet().setModelParameterValue("tilt_x", tilt_x);
  model_old->getModelParameterSet().setModelParameterValue("tilt_y", tilt_y);
  model_fast->getModelParameterSet().setModelParameterValue("tilt_x", tilt_x);
  model_fast->getModelParameterSet().setModelParameterValue("tilt_y", tilt_y);

  // grid evaluation

  std::vector<DataStructs::DimensionRange> integral_range_thetaxy;
  std::vector<DataStructs::DimensionRange> integral_range_thetaphi;

  DataStructs::DimensionRange temprange(0.0, 0.0);
  integral_range_thetaxy.push_back(temprange);
  integral_range_thetaxy.push_back(temprange);
  integral_range_thetaphi.push_back(temprange);
  integral_range_thetaphi.push_back(temprange);

  integral_range_thetaxy[0].range_low = -0.01;
  integral_range_thetaxy[0].range_high = 0.01;

  integral_range_thetaxy[1].range_low = -0.01;
  integral_range_thetaxy[1].range_high = 0.01;

  integral_range_thetaphi[0].range_low = 0.0;
  integral_range_thetaphi[0].range_high = 0.01;
  integral_range_thetaphi[1].range_low = -TMath::Pi();
  integral_range_thetaphi[1].range_high = TMath::Pi();

  /*integral_range_thetaphi[0].range_low = atan(
                  sqrt(
                                  pow(integral_range_thetaxy[0].range_low, 2.0)
                                                  +
  pow(integral_range_thetaxy[1].range_low, 2.0)));
  integral_range_thetaphi[0].range_high = atan(
                  sqrt(
                                  pow(integral_range_thetaxy[0].range_high, 2.0)
                                                  +
  pow(integral_range_thetaxy[1].range_low, 2.0)));
  integral_range_thetaphi[1].range_low = atan2(
                  integral_range_thetaxy[1].range_low,
  integral_range_thetaxy[0].range_low); integral_range_thetaphi[1].range_high =
  atan2( integral_range_thetaxy[1].range_high,
                  integral_range_thetaxy[0].range_low);*/

  std::cout << "x y: " << integral_range_thetaxy[0].range_low << " - "
            << integral_range_thetaxy[0].range_high << " "
            << integral_range_thetaxy[1].range_low << " - "
            << integral_range_thetaxy[1].range_high << std::endl;
  std::cout << "theta phi: " << integral_range_thetaphi[0].range_low << " - "
            << integral_range_thetaphi[0].range_high << " "
            << integral_range_thetaphi[1].range_low << " - "
            << integral_range_thetaphi[1].range_high << std::endl;

  std::cout << "fast model: "
            << model_fast->Integral(integral_range_thetaxy, 1e-10) << std::endl;
  std::cout << "old model: "
            << model_old->Integral(integral_range_thetaphi, 1e-10) << std::endl;
}
