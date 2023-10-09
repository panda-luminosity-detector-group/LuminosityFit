#include "TCanvas.h"
#include <string>

void plotEfficiency(std::string inFileName, std::string outName) {
  TFile *f = new TFile(inFileName.c_str());
  auto acceptance = f->Get<PndLmdAcceptance>("lmd_acceptance");
  auto eff2D = acceptance->getAcceptance2D();
  gStyle->SetPalette(1);
  {

    auto canvas = TCanvas("canvas", "canvas", 1920, 1080);
    eff2D->Draw("lego2");
    canvas.Print((outName + "-lego2.png").c_str());
  }
  {

    auto canvas = TCanvas("canvas", "canvas", 1080, 1080);
    eff2D->Draw("colz");
    canvas.Print((outName + "-colz.png").c_str());
  }
}

void plotEfficiency() { plotEfficiency("lmd_acc_data.root", "acceptance"); }