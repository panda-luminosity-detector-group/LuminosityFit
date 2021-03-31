#ifndef TOOLS_PBARPELASTICSCATTERINGEVENTGENERATOR_H_
#define TOOLS_PBARPELASTICSCATTERINGEVENTGENERATOR_H_

#include "data/PndLmdAngularData.h"
#include "model/PndLmdDPMAngModel1D.h"
#include "model/PndLmdModelFactory.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TMath.h"
#include "TParticle.h"
#include "TRandom3.h"

#include "boost/property_tree/ptree.hpp"

namespace PbarPElasticScattering {
  std::pair<TTree *, double> generateEvents(double momentum,
                                          unsigned int num_events,
                                          double theta_min_in_mrad,
                                          double theta_max_in_mrad,
                                          double phi_min_in_rad,
                                          double phi_max_in_rad,
                                          unsigned int seed
                                          ) {
  PndLmdModelFactory model_factory;

  boost::property_tree::ptree base_model_opt;
  base_model_opt.put("fit_dimension", 1);
  base_model_opt.put("momentum_transfer_active", true);
  base_model_opt.put("acceptance_correction_active", false);
  base_model_opt.put("resolution_smearing_active", false);
  base_model_opt.put("dpm_elastic_parts", "ALL");

  boost::property_tree::ptree correct_model_opt(base_model_opt);
  correct_model_opt.put("theta_t_trafo_option", "APPROX");
  PndLmdAngularData data;
  data.setLabMomentum(momentum);

  std::shared_ptr<Model> correct_model =
      model_factory.generateModel(correct_model_opt, data);

  double t_min = model_factory.getMomentumTransferFromTheta(
      momentum, theta_min_in_mrad / 1000.0);
  double t_max = model_factory.getMomentumTransferFromTheta(
      momentum, theta_max_in_mrad / 1000.0);

  TDatabasePDG *pdg = TDatabasePDG::Instance();
  double mass_proton = pdg->GetParticle(2212)->Mass();
  double fPhiMin = phi_min_in_rad;
  double fPhiMax = phi_max_in_rad;
  double plab = momentum;
  double Elab = std::sqrt(std::pow(mass_proton, 2) + std::pow(plab, 2));
  double s = 2.0 * std::pow(mass_proton, 2) + 2.0 * mass_proton * Elab;
  double sqrts = std::sqrt(s);
  double Ecms = sqrts / 2.;
  double Pcms2 = s / 4.0 - std::pow(mass_proton, 2);
  double Pcms = std::sqrt(Pcms2);

  double temp_t_max = 4. * Pcms2;
  if (temp_t_max < t_max) {
    std::cout << "specified tmax=" << t_max << " above limit=" << temp_t_max
              << "\n";
    t_max = temp_t_max;
  }

  std::vector<DataStructs::DimensionRange> integral_ranges;
  DataStructs::DimensionRange dr(t_min, t_max);
  integral_ranges.push_back(dr);

  double integral = correct_model->Integral(integral_ranges, 1e-5);
  integral *= (phi_max-phi_min)/2*pi;
  std::cout << "Integrated total elastic cross section in theta range ["
            << theta_min_in_mrad << " - " << theta_max_in_mrad << "] -> t ["
            << t_min << " - " << t_max << "] is " << integral << " mb"
            << std::endl;

  if (num_events == 0) {
    return std::make_pair(nullptr, integral);
  }

  TClonesArray ca("TParticle", 2);
  TClonesArray *pars = &ca;

  TTree *tree = new TTree("data", "data");
  tree->Branch("Particles", &pars);

  std::cout << "using seed: " << seed << "\n";
  TRandom3 RandomGen(seed);
  TRandom3 RandomGen2(seed + 123456);
  TRandom3 RandomGen3(seed + 1234);

  double func_max(0.0);
  std::cout << "t_min: " << t_min << "\n";
  std::cout << "t_max: " << t_max << "\n";

  for (unsigned int i = 0; i < 100000; ++i) {
    mydouble t = RandomGen.Uniform(t_min, t_max);
    double funcval = correct_model->eval(&t);
    if (funcval > func_max) {
      func_max = funcval;
    }
  }
  func_max *= 1.1;
  std::cout << "function maximum was determined as: " << func_max << "\n";
  std::cout << "generating " << num_events << " ....\n";

  TVector3 boost_vector(0.0, 0.0, Pcms / Ecms);
  using namespace std::chrono;
  duration<double> time_span_correct;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  for (unsigned int i = 0; i < num_events; ++i) {
    mydouble t = RandomGen.Uniform(t_min, t_max);
    mydouble funcval = correct_model->eval(&t);
    double randval = RandomGen2.Uniform(0.0, func_max);
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

    double phi = RandomGen3.Uniform(fPhiMin, fPhiMax);
    double costheta = 1.0 - 0.5 * t / Pcms2;
    double sintheta = std::sqrt(std::abs(1.0 - std::pow(costheta, 2)));

    double pz = Pcms * costheta;
    double pt = Pcms * sintheta;
    double px = pt * std::cos(phi);
    double py = pt * std::sin(phi);

    TLorentzVector vertex(0, 0, 0, 0);

    double E(
        std::sqrt(mass_proton * mass_proton + px * px + py * py + pz * pz));
    TLorentzVector mom_pbar(px, py, pz, E);
    TLorentzVector mom_p(-px, -py, -pz, E);
    // boost to lab
    mom_pbar.Boost(boost_vector);
    mom_p.Boost(boost_vector);
    // Gamma = (Elab + AMProton) / SqrtS

    new (ca[0]) TParticle(-2212, 1, 0, 0, 0, 0, mom_pbar, vertex);
    new (ca[1]) TParticle(2212, 1, 0, 0, 0, 0, mom_p, vertex);

    tree->Fill();
    ca.Delete();
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  time_span_correct += duration_cast<duration<double>>(t2 - t1);
  std::cout << "Finished generation! Time spend on generation per event: "
            << time_span_correct.count() / num_events << "\n";
  return std::make_pair(tree, integral);
}

} // namespace PbarPElasticScattering

#endif /* TOOLS_PBARPELASTICSCATTERINGEVENTGENERATOR_H_ */
