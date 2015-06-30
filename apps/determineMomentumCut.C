void determineMomentumCut(TString dirname, unsigned int entries =100000) {
  TChain data_tree("cbmsim");

  TClonesArray* track_array;

  data_tree.Add(dirname + "/Lumi_TrksQA*.root");

  track_array = new TClonesArray("PndLmdTrackQ");
  data_tree.SetBranchAddress("LMDTrackQ", &track_array);
  data_tree.SetBranchStatus("LMDTrackQ*", 1);
  

  TH2D *mom = new TH2D("mom_cut_hist", "", 1000, -0.005, 0.005, 10000, 0.0, 0.2);

  if(data_tree.GetEntries() < entries)
    entries = data_tree.GetEntries();  

  for(unsigned int i = 0; i < entries; i++) {
    data_tree.GetEntry(i);

   //loop over track array
    for (Int_t iN = 0; iN < track_array->GetEntries(); iN++) {
      PndLmdTrackQ *track = (PndLmdTrackQ*)track_array->At(iN);
      // track status < 0 is a missed track
      if (0 > track->GetTrkRecStatus())
        continue;
      mom->Fill(track->GetMCtheta() - track->GetIPtheta(), (track->GetIPmom() - track->GetMCmom())/track->GetMCmom());
    }
  }

  mom->Draw("colz");
  TLine *cut = new TLine(-0.005, 0.0003, 0.005, 0.0003);
  cut->SetLineColor(2);
  cut->SetLineWidth(2.0);
  cut->Draw();
}
