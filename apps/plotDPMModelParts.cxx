#include "PndLmdPlotter.h"
#include "model/PndLmdModelFactory.h"

#include <cmath>
#include <sstream>
#include <stdlib.h>
#include <utility>

#include "TCanvas.h"
#include "TColor.h"
#include "TGaxis.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TROOT.h"
#include "TStyle.h"

int main(int argc, char *argv[]) {
  // some style stuff
  TPad foo; // never remove this line :-)))
  if (1) {
    gROOT->SetStyle("Plain");
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;
    Double_t stops[NRGBs] = {0.00, 0.34, 0.61, 0.84, 1.00};
    Double_t red[NRGBs] = {0.00, 0.00, 0.87, 1.00, 0.51};
    Double_t green[NRGBs] = {0.00, 0.81, 1.00, 0.20, 0.00};
    Double_t blue[NRGBs] = {0.51, 1.00, 0.12, 0.00, 0.00};
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
    gStyle->SetTitleFont(10 * 13 + 2, "xyz");
    gStyle->SetTitleSize(0.06, "xyz");
    gStyle->SetTitleOffset(1.3, "y");
    gStyle->SetTitleOffset(1.3, "z");
    gStyle->SetLabelFont(10 * 13 + 2, "xyz");
    gStyle->SetLabelSize(0.06, "xyz");
    gStyle->SetLabelOffset(0.009, "xyz");
    gStyle->SetPadBottomMargin(0.16);
    gStyle->SetPadTopMargin(0.16);
    gStyle->SetPadLeftMargin(0.16);
    gStyle->SetPadRightMargin(0.16);
    gStyle->SetOptTitle(1);
    gStyle->SetOptStat(1);
    gROOT->ForceStyle();
    gStyle->SetFrameFillColor(0);
    gStyle->SetFrameFillStyle(0);
    TGaxis::SetMaxDigits(3);
  }

  if (argc == 2) {
    double plab = atof(argv[1]);

    PndLmdModelFactory model_factory;

    LumiFit::PndLmdFitModelOptions fit_op_full;

    shared_ptr<Model1D> model =
        model_factory.generate1DModel(fit_op_full, plab);
    if (model->init()) {
      std::cout << "Error: not all parameters have been set!" << std::endl;
    }
    std::vector<DataStructs::DimensionRange> integral_region;
    DataStructs::DimensionRange th_int_range(0.003, 0.09);
    integral_region.push_back(th_int_range);
    std::cout << "cross section integral in mb: "
              << model->Integral(integral_region, 1e-3) << std::endl;

    const double theta_min = 0.0025;
    const double theta_max = 0.01;

    const double log_scale = true;

    LumiFit::PndLmdPlotter plotter;
    DataStructs::DimensionRange plot_range(theta_min, theta_max);

    // model part choices are: COUL, INT, HAD, HAD_RHO_B_SIGTOT,
    // ALL_RHO_B_SIGTOT, ALL

    TGraphAsymmErrors *full_model_graph =
        plotter.generateDPMModelPartGraph(plab, LumiFit::ALL, plot_range);
    TGraphAsymmErrors *coul_model_graph =
        plotter.generateDPMModelPartGraph(plab, LumiFit::COUL, plot_range);
    TGraphAsymmErrors *had_model_graph =
        plotter.generateDPMModelPartGraph(plab, LumiFit::HAD, plot_range);
    TMultiGraph *model_graph = new TMultiGraph();
    // change from rad to mrad
    for (int ipoint = 0; ipoint < full_model_graph->GetN(); ipoint++) {
      double x, y;
      full_model_graph->GetPoint(ipoint, x, y);
      full_model_graph->SetPoint(ipoint, x * 1e3, y);
    }
    for (int ipoint = 0; ipoint < coul_model_graph->GetN(); ipoint++) {
      double x, y;
      coul_model_graph->GetPoint(ipoint, x, y);
      coul_model_graph->SetPoint(ipoint, x * 1e3, y);
    }
    for (int ipoint = 0; ipoint < had_model_graph->GetN(); ipoint++) {
      double x, y;
      had_model_graph->GetPoint(ipoint, x, y);
      had_model_graph->SetPoint(ipoint, x * 1e3, y);
    }

    model_graph->Add(full_model_graph);
    model_graph->Add(coul_model_graph);
    model_graph->Add(had_model_graph);

    TCanvas c("c", "", 600, 600);
    c.SetLogy(log_scale);

    // determine y axis range (mainly required for interference part)
    model_graph->Draw("AC");
    gPad->Update();
    double top_pos = gPad->GetUymax();
    if (log_scale)
      top_pos = pow(10, gPad->GetUymax());
    gPad->Update();
    double bottom_pos = gPad->GetUymin();
    if (log_scale)
      bottom_pos = pow(10, gPad->GetUymin());
    model_graph->GetXaxis()->SetRangeUser(theta_min, theta_max);
    model_graph->GetYaxis()->SetRangeUser(bottom_pos, top_pos);

    model_graph->GetXaxis()->SetTitle("#theta [mrad]");
    model_graph->GetYaxis()->SetTitle("d#sigma/d#theta");

    TLegend *legend;
    if (plab < 6) {
      legend = new TLegend(0.6, 0.65, 0.88, 0.85);
    } else {
      legend = new TLegend(0.2, 0.2, 0.48, 0.4);
    }
    legend->AddEntry(full_model_graph, "#sigma_{total}", "l");
    legend->AddEntry(coul_model_graph, "#sigma_{coulomb}", "l");
    legend->AddEntry(had_model_graph, "#sigma_{hadron}", "l");
    legend->SetFillColor(0);
    legend->SetBorderSize(0);

    full_model_graph->SetLineWidth(2);
    // full_model_graph->Draw("AC");
    coul_model_graph->SetLineWidth(2);
    coul_model_graph->SetLineColor(2);
    // coul_model_graph->Draw("CSAME");
    had_model_graph->SetLineWidth(2);
    had_model_graph->SetLineColor(9);
    // had_model_graph->Draw("CSAME");
    legend->Draw();

    std::stringstream strstream;
    strstream.precision(3);

    strstream << "DPMModels_" << plab << ".pdf";
    c.SaveAs(strstream.str().c_str());
    c.Print(strstream.str().c_str());
  }
  return 0;
}
