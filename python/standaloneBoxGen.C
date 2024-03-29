void standaloneBoxGen(double plab, int nevents, double fThetaMinInMilliRad,
                      double fThetaMaxInMilliRad, double fPhiMinInRad = 0,
                      double fPhiMaxInRad = 2 * TMath::Pi(),
                      TString output_filename = "gen_mc.root",
                      double seed = 123456.,
                      bool use_recoil_correction = true) {
  double mp = 0.938272;

  TClonesArray ca("TParticle", 1);
  TClonesArray *pars = &ca;

  TFile *f = new TFile(output_filename, "RECREATE");

  TTree *t = new TTree("data", "data");
  t->Branch("Particles", &pars);

  gRandom->SetSeed(seed);

  // boost::progress_display show_progress(nevents);

  double Elab = sqrt(mp * mp + plab * plab);
  double S = 2 * mp * mp + 2 * mp * Elab;
  double pcm2 = S / 4. - mp * mp;
  double beta_lab_cms = plab / (mp + Elab);
  double gamma = (Elab + mp) / sqrt(S);

  if (fPhiMaxInRad > 2 * TMath::Pi()) {
    cout << "phi max is too large. capping to 2 pi\n";
    fPhiMaxInRad = 2 * TMath::Pi();
  }

  cout << "generating " << nevents << " ....\n";
  for (unsigned int i = 0; i < nevents; i++) {
    // double theta = tmp->GetRandom(gen_range_low, gen_range_high) / 1000.0;
    // double phi = gRandom->Uniform(-C_PI, C_PI);

    double theta = gRandom->Uniform(fThetaMinInMilliRad, fThetaMaxInMilliRad) / 1000.0;
    double phi = gRandom->Uniform(fPhiMinInRad, fPhiMaxInRad);

    double Er = 0.0;
    // if recoil energy should be taken into account
    if (use_recoil_correction) {
      // read lmd note/tdr for a derivation of this formula
      double signum = -1.0;
      double denominator = plab * cos(theta) - beta_lab_cms * Elab;
      if (0.0 > denominator)
        signum = 1.0;

      double tm =
          -2.0 * pcm2 *
          (1.0 +
           signum /
               sqrt(1 + pow(plab * sin(theta) / (gamma * denominator), 2.0)));

      Er = -tm / 2.0 / mp;
    }

    double pabs = sqrt(pow(Elab - Er, 2.0) - pow(mp, 2.0));

    double pz = pabs * TMath::Cos(theta);
    double pt = pabs * TMath::Sin(theta);

    double px = pt * TMath::Cos(phi);
    double py = pt * TMath::Sin(phi);

    TLorentzVector vertex(0, 0, 0, 0);
    TLorentzVector mom(px, py, pz, sqrt(mp * mp + pabs * pabs));

    TParticle p(-2212, 1, 0, 0, 0, 0, mom, vertex);
    new (ca[0]) TParticle(-2212, 1, 0, 0, 0, 0, mom, vertex);

    t->Fill();
    ca.Delete();

    // ++show_progress;
  }

  t->Write();
  f->Close();
  cout << "Successfully finished box data generation!" << endl;
}
