void plotLumiHist(std::string inFileName, std::string outname) {
  TFile *f = new TFile(inFileName.c_str());

  auto dataBundle = (PndLmdFitDataBundle *)f->Get("lmd_elastic_data_bundle");
  auto elasticDataBundle = dataBundle->getCurrentElasticBundle();

  TH2D *lmdHist = elasticDataBundle.get2DHistogram();

  TCanvas *c = new TCanvas("c1", "c1", 640, 640);
  if (lmdHist) {
    lmdHist->SetTitle("LMD Fitted Data");
    lmdHist->Draw("colz");
  } else {
    cout << "nej on the hist\n";
  }
  c->Print(outname.c_str());
}