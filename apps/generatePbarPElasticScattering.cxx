/*
 * calculateElasticCrossSection.cxx
 *
 *  Created on: Apr 7, 2014
 *      Author: steve
 */

#include "model/PndLmdModelFactory.h"
#include "model/PndLmdDPMAngModel1D.h"
#include "data/PndLmdAngularData.h"

#include <utility>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>
#include <fstream>

#include "TFile.h"
#include "TDatabasePDG.h"
#include "TMath.h"
#include "TParticle.h"
#include "TRandom.h"
#include "TClonesArray.h"

#include "boost/property_tree/ptree.hpp"

void calculateElasticCrossSection(double momentum, unsigned int num_events,
    double theta_min_in_mrad, double theta_max_in_mrad, unsigned int seed,
    const std::string &output_filepath) {
  PndLmdModelFactory model_factory;

  boost::property_tree::ptree base_model_opt;
  base_model_opt.put("fit_dimension", 1);
  base_model_opt.put("momentum_transfer_active", false);
  base_model_opt.put("acceptance_correction_active", false);
  base_model_opt.put("resolution_smearing_active", false);
  base_model_opt.put("dpm_elastic_parts", "ALL");

  boost::property_tree::ptree correct_model_opt(base_model_opt);
  correct_model_opt.put("theta_t_trafo_option", "APPROX");
  PndLmdAngularData data;
  data.setLabMomentum(momentum);

  std::shared_ptr<Model> correct_model = model_factory.generateModel(
      correct_model_opt, data);
  std::vector<DataStructs::DimensionRange> integral_ranges;
  DataStructs::DimensionRange dr(theta_min_in_mrad / 1000.0,
      theta_max_in_mrad / 1000.0);
  integral_ranges.push_back(dr);

  double integral = correct_model->Integral(integral_ranges, 0.001);
  std::cout << "Integrated total elastic cross section in theta range ["
      << theta_min_in_mrad << " - " << theta_max_in_mrad << "] is " << integral
      << " mb" << std::endl;

  if(num_events == 0) {
    std::ofstream myfile;
    myfile.open ("elastic_cross_section.txt");
    myfile << integral;
    myfile.close();
    return;
  }

  TDatabasePDG *pdg = TDatabasePDG::Instance();
  double mass_proton = pdg->GetParticle(2212)->Mass();
  double fPhiMin = 0.0;
  double fPhiMax = 360.0;
  double plab = momentum;
  double Elab = std::sqrt(std::pow(mass_proton, 2) + std::pow(plab, 2));
  double s = 2.0 * std::pow(mass_proton, 2) + 2.0 * mass_proton * Elab;
  double sqrts = std::sqrt(s);
  double Ecms = sqrts / 2.;
  double Pcms2 = std::pow(Ecms, 2) - std::pow(mass_proton, 2);
  double Pcms = std::sqrt(Pcms2);

  double t_min = ((PndLmdDPMAngModel1D*)correct_model.get())->getMomentumTransferFromTheta(theta_min_in_mrad / 1000.0);
  double t_max = ((PndLmdDPMAngModel1D*)correct_model.get())->getMomentumTransferFromTheta(theta_max_in_mrad / 1000.0);
  //double t_max = -4. * std::pow(Pcms, 2);

  TClonesArray ca("TParticle", 2);
  TClonesArray *pars = &ca;

  TFile f(output_filepath.c_str(), "RECREATE");

  TTree *tree = new TTree("data", "data");
  tree->Branch("Particles", &pars);

  std::cout << "using seed: " << seed << "\n";
  gRandom->SetSeed(seed);

  double func_max(0.0);
  std::cout << "t_min: " << t_min << "\n";
  std::cout << "t_max: " << t_max << "\n";
  for (unsigned int i = 0; i < 100000; ++i) {
    mydouble t = gRandom->Uniform(t_min, t_max);
    double funcval = correct_model->eval(&t);
    if (funcval > func_max) {
      func_max = funcval;
    }
  }
  func_max *= 1.1;
  std::cout << "function maximum was determined as: " << func_max << "\n";
  std::cout << "generating " << num_events << " ....\n";
  TVector3 boost_vector = TLorentzVector(0, 0, Pcms, Ecms).BoostVector();
  using namespace std::chrono;
  duration<double> time_span_correct;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  for (unsigned int i = 0; i < num_events; ++i) {
    double phi = gRandom->Uniform(fPhiMin, fPhiMax) * TMath::DegToRad();
    mydouble t = gRandom->Uniform(t_min, t_max);
    double funcval = correct_model->eval(&t);
    double randval = gRandom->Uniform(0.0, func_max);
    if (funcval > func_max) {
      std::cout
          << "Error: function maximum was determined not large enough...\n";
      std::cout << "function evaluation: " << funcval << "\n";
      std::cout << "function maximum was determined as: " << func_max << "\n";
    }
    if (randval > funcval) {
      --i;
      continue;
    }
    double theta = std::acos(1.0 - 0.5 * (-t / Pcms2));

    double pz = Pcms * TMath::Cos(theta);
    double pt = Pcms * TMath::Sin(theta);
    double px = pt * TMath::Cos(phi);
    double py = pt * TMath::Sin(phi);

    TLorentzVector vertex(0, 0, 0, 0);

    TLorentzVector mom_pbar(px, py, pz, Ecms);
    TLorentzVector mom_p(-px, -py, -pz, Ecms);
    // boost to lab
    mom_pbar.Boost(boost_vector);
    mom_p.Boost(boost_vector);

    new (ca[0]) TParticle(-2212, 1, 0, 0, 0, 0, mom_pbar, vertex);
    new (ca[1]) TParticle(2212, 1, 0, 0, 0, 0, mom_p, vertex);

    tree->Fill();
    ca.Delete();
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  time_span_correct += duration_cast<duration<double>>(t2 - t1);
  std::cout << "Finished generation! Time spend on generation per event: "
      << time_span_correct.count() / num_events << "\n";
  std::cout << "writing to file " << output_filepath << " ...\n";
  tree->Write();
  f.Close();
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "[pbar momentum]" << std::endl;
  std::cout << "[number of events]" << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-l [lower theta boundary in mrad] (default: 2mrad)"
      << std::endl;
  std::cout << "-u [upper theta boundary in mrad] (default: 12mrad)"
      << std::endl;
  std::cout << "-o [output filepath] (default: dpmgen.root)" << std::endl;
  std::cout << "-s [random seed] (default: 12345)" << std::endl;
}

int main(int argc, char* argv[]) {
  double lower_bound = 2.0;
  double upper_bound = 12.0;
  std::string output_filepath = "dpmgen.root";
  unsigned int seed = 12345;
  int c;

  while ((c = getopt(argc, argv, "hl:u:o:s:")) != -1) {
    switch (c) {
    case 'o':
      output_filepath = optarg;
      break;
    case 's':
      seed = std::stoul(optarg);
      break;
    case 'l':
      lower_bound = std::stod(optarg);
      break;
    case 'u':
      upper_bound = std::stod(optarg);
      break;
    case '?':
      if (optopt == 'l' || optopt == 'u' || optopt == 'o' || optopt == 's')
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

  if (argc - optind == 2) {
    double momentum = std::stod(argv[optind]);
    unsigned int num_events = std::stoul(argv[optind + 1]);
    calculateElasticCrossSection(momentum, num_events, lower_bound, upper_bound,
        seed, output_filepath);
  } else
    displayInfo();

  return 0;
}
