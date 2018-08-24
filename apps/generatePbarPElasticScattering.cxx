/*
 * calculateElasticCrossSection.cxx
 *
 *  Created on: Apr 7, 2014
 *      Author: steve
 */

#include "model/PndLmdModelFactory.h"
#include "data/PndLmdAngularData.h"

#include <utility>
#include <vector>
#include <iostream>
#include <chrono>

#include "TFile.h"

#include "boost/property_tree/ptree.hpp"

void calculateElasticCrossSection(double momentum, double lower_bound,
    double upper_bound) {
  PndLmdModelFactory model_factory;

  boost::property_tree::ptree base_model_opt;
  base_model_opt.put("fit_dimension", 1);
  base_model_opt.put("momentum_transfer_active", false);
  base_model_opt.put("acceptance_correction_active", false);
  base_model_opt.put("resolution_smearing_active", false);
  base_model_opt.put("dpm_elastic_parts", "ALL");

  boost::property_tree::ptree correct_model_opt(base_model_opt);
          correct_model_opt.put("theta_t_trafo_option", "CORRECT");
  PndLmdAngularData data;
  data.setLabMomentum(momentum);


  std::shared_ptr<Model> correct_model = model_factory.generateModel(correct_model_opt, data);
  std::vector<DataStructs::DimensionRange> integral_ranges;
  DataStructs::DimensionRange dr(lower_bound / 1000.0, upper_bound / 1000.0);
  integral_ranges.push_back(dr);

  using namespace std::chrono;
  duration<double> time_span_correct;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  double integral = correct_model->Integral(integral_ranges, 0.001);
  high_resolution_clock::time_point t2 = high_resolution_clock::now();

  time_span_correct += duration_cast<duration<double>>(t2 - t1);
  std::cout << "Integrated total elastic cross section in theta range ["
      << lower_bound << " - " << upper_bound << "] is " << integral << " mb"
      << std::endl;

/*
  for(unsigned int i = 0; i < steps; i++) {
    mydouble x = 0.01*i/steps;
    mydouble true_value = correct_model->eval(&x);

  }

  TFile *f = new TFile("data.root", "RECREATE");
  g->Write("correct");
  g1->Write("approx");
  diff->Write("reldiff");
  f->Close();*/

  std::cout<<"Finished! Time spend on generation per event: "<<time_span_correct.count()<<std::endl;
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "-m [pbar momentum]" << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-l [lower theta boundary in mrad] (default: 2mrad)"
      << std::endl;
  std::cout << "-u [upper theta boundary in mrad] (default: 12mrad)"
      << std::endl;
}

int main(int argc, char* argv[]) {
  bool is_mom_set = false;
  double momentum = -1.0;
  double lower_bound = 2.0;
  double upper_bound = 12.0;
  int c;

  while ((c = getopt(argc, argv, "hm:l:u:")) != -1) {
    switch (c) {
      case 'm':
        momentum = atof(optarg);
        is_mom_set = true;
        break;
      case 'l':
        lower_bound = atof(optarg);
        break;
      case 'u':
        upper_bound = atof(optarg);
        break;
      case '?':
        if (optopt == 'm' || optopt == 'l' || optopt == 'u')
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

  if (is_mom_set)
    calculateElasticCrossSection(momentum, lower_bound, upper_bound);
  else
    displayInfo();

  return 0;
}
