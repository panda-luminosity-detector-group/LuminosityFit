/*
 * PndLmdDPMModelParametrizator.cxx
 *
 *  Created on: Jan 22, 2013
 *      Author: steve
 */

#include "PndLmdDPMModelParametrization.h"

#include <cmath>
#include <iostream>

#include "TDatabasePDG.h"

PndLmdDPMModelParametrization::PndLmdDPMModelParametrization(
    ModelParSet &model_par_set_)
    : Parametrization(model_par_set_) {
  initParameters();
}

PndLmdDPMModelParametrization::~PndLmdDPMModelParametrization() {
  // TODO Auto-generated destructor stub
}

void PndLmdDPMModelParametrization::initParameters() {
  p_lab = model_par_set.getModelParameter("p_lab");
  E_lab = model_par_set.getModelParameter("E_lab");
  S = model_par_set.getModelParameter("S");
  pcm2 = model_par_set.getModelParameter("pcm2");
  gamma = model_par_set.getModelParameter("gamma");
  beta = model_par_set.getModelParameter("beta");
  beta_lab_cms = model_par_set.getModelParameter("beta_lab_cms");
  A1 = model_par_set.getModelParameter("A1");
  T1 = model_par_set.getModelParameter("T1");
  A2 = model_par_set.getModelParameter("A2");
  T2 = model_par_set.getModelParameter("T2");
  A3 = model_par_set.getModelParameter("A3");
  rho = model_par_set.getModelParameter("rho");
  b = model_par_set.getModelParameter("b");
  sigma_tot = model_par_set.getModelParameter("sigma_tot");

  dependency_parameters.push_back(p_lab);

  p_lab->setConnectionTo(E_lab);

  p_lab->setConnectionTo(S);
  p_lab->setConnectionTo(pcm2);
  p_lab->setConnectionTo(gamma);
  p_lab->setConnectionTo(beta);
  p_lab->setConnectionTo(beta_lab_cms);

  p_lab->setConnectionTo(A1);
  p_lab->setConnectionTo(T1);
  p_lab->setConnectionTo(A2);
  p_lab->setConnectionTo(T2);
  p_lab->setConnectionTo(A3);
  p_lab->setConnectionTo(rho);
  p_lab->setConnectionTo(b);
  p_lab->setConnectionTo(sigma_tot);
}

void PndLmdDPMModelParametrization::adjustStrictParameters() {
  TDatabasePDG *pdg = TDatabasePDG::Instance();
  double mass_proton = pdg->GetParticle(-2212)->Mass();
  E_lab->setValue(
      sqrt(mass_proton * mass_proton + p_lab->getValue() * p_lab->getValue()));
  S->setValue(2 * mass_proton * mass_proton +
              2 * mass_proton * E_lab->getValue());
  pcm2->setValue(S->getValue() / 4. - mass_proton * mass_proton);
  beta->setValue(p_lab->getValue() /
                 E_lab->getValue()); // this is beta of beam inside lab frame
                                     // (used only for cross sections)
  beta_lab_cms->setValue(
      p_lab->getValue() /
      (mass_proton +
       E_lab->getValue())); // this is beta to go from lab to cms system
  gamma->setValue(
      (E_lab->getValue() + mass_proton) /
      sqrt(S->getValue())); // this boost gamma from lab to cms system
}

void PndLmdDPMModelParametrization::adjustNonStrictParameters() {
  double A1_val = 115.0 + 650.0 * exp(-p_lab->getValue() / 4.08);
  double T1_val = 0.0899;
  double A2_val = 0.0687 + 0.307 * exp(-p_lab->getValue() / 2.367);
  double T2_val = -2.979 + 3.353 * exp(-p_lab->getValue() / 483.4);
  double A3_val = 0.8372 + 39.53 * exp(-p_lab->getValue() / 0.765);
  A1->setValue(A1_val);
  T1->setValue(T1_val);
  A2->setValue(A2_val);
  T2->setValue(T2_val);
  A3->setValue(A3_val);

  // T2 = -2.979 + 3.353 * exp(-Plab / 0.67009);
  // A3 = 0.11959 + 3.86474 * exp(-Plab / 0.765);

  rho->setValue(-sqrt(A3_val) / (sqrt(A1_val) * (1.0 - A2_val)));
  b->setValue((A1_val / T1_val + A1_val * pow(A2_val, 2.0) / T2_val -
               2.0 * A1_val * A2_val * (1. / 2. / T1_val + 1. / 2. / T2_val) +
               A3_val / T2_val) /
              (A1_val * pow(1.0 - A2_val, 2.0) + A3_val));
  sigma_tot->setValue(4.0 * sqrt(3.1416 * A1_val * 0.1 / pow(5.067, 2.0)) *
                      (1.0 - A2_val) * 10.0);

  /* std::cout << "**************************************************"
   << std::endl;
   std::cout << "Plab: " << p_lab.getValue() << std::endl;
   std::cout << "--------------------------------------------------"
   << std::endl;
   std::cout << "A1: " << A1.getValue() << std::endl;
   std::cout << "T1: " << T1.getValue() << std::endl;
   std::cout << "A2: " << A2.getValue() << std::endl;
   std::cout << "T2: " << T2.getValue() << std::endl;
   std::cout << "A3: " << A3.getValue() << std::endl;
   std::cout << "--------------------------------------------------"
   << std::endl;
   std::cout << "Sigtot: " << sigma_tot.getValue() << std::endl;
   std::cout << "b: " << b.getValue() << std::endl;
   std::cout << "rho: " << rho.getValue() << std::endl;
   std::cout << "**************************************************"
   << std::endl;*/
}

void PndLmdDPMModelParametrization::parametrize() {
  // this function will be called only once in the beginning
  adjustStrictParameters();
  adjustNonStrictParameters();
}
