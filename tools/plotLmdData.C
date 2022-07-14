void plotLmdData(std::string inFileName, std::string outname) {

  TFile *f = new TFile(inFileName.c_str());

  auto histNames = {"mc_th", "mc_acc", "reco"};

  for (auto histName : histNames) {

    auto mcData = (PndLmdAngularData *)f->Get(TString(histName));

    if (mcData) {

      TH2D *hist = mcData->get2DHistogram();
      TCanvas *c = new TCanvas("c1", "c1", 640, 640);
      if (hist) {
        hist->SetTitle(histName);
        hist->Draw("colz");
      } else {
        cout << "nej on the hist\n";
      }
      c->Print(TString(outname) + TString("-") + TString(histName) +
               TString(".png"));
    }

    else {
      cout << "nej on the data\n";
    }
  }
}