#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdPlotter.h"
#include "fit/PndLmdLumiFitResult.h"
#include "data/PndLmdAcceptance.h"
#include "model/PndLmdROOTDataModel1D.h"

#include <iostream>               // for std::cout
#include <utility>
#include <vector>
#include <sstream>

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TGaxis.h"

double meanSignedAverageError(const PndLmdAcceptance& acc) {
  std::cout << "calculating mean signed error for acceptance...\n";
  TEfficiency *eff = acc.getAcceptance2D(); // false = angular acceptance
  TCanvas c;
  eff->Draw("colz");
  c.Update();

  TH2D* acchist2d = (TH2D*) eff->GetPaintedHistogram();

  double dx = (acchist2d->GetXaxis()->GetXmax()
      - acchist2d->GetXaxis()->GetXmin()) / acchist2d->GetXaxis()->GetNbins();
  double dy = (acchist2d->GetYaxis()->GetXmax()
      - acchist2d->GetYaxis()->GetXmin()) / acchist2d->GetYaxis()->GetNbins();

  double mean(0.0);
  unsigned int counter(0);
  for (unsigned int ix = 1; ix < acchist2d->GetXaxis()->GetNbins() + 1; ++ix) {
    for (unsigned int iy = 1; iy < acchist2d->GetYaxis()->GetNbins() + 1;
        ++iy) {
      ++counter;
      int bin = eff->FindFixBin(
          acchist2d->GetXaxis()->GetXmin() + (0.5 + ix) * dx,
          acchist2d->GetYaxis()->GetXmin() + (0.5 + iy) * dy);
      double err_low = eff->GetEfficiencyErrorLow(bin);
      double err_high = eff->GetEfficiencyErrorUp(bin);

      if (eff->GetEfficiency(bin) != 0.0) {
        //std::cout<<err_low<< " " << err_high<<std::endl;
        mean += err_high - err_low;
      }
    }
  }

  return mean / counter;
}

void analyzeAcceptances(std::vector<std::string> &acceptance_files) {
  std::cout << "Generating acceptance comparison plots ....\n";

  // ================================ BEGIN CONFIG ================================ //
  // PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
  // you can fine tune it and overwrite the default values
    gStyle->SetPadRightMargin(0.07);
    gStyle->SetPadLeftMargin(0.125);
    gStyle->SetPadBottomMargin(0.126);
    gStyle->SetPadTopMargin(0.03);
    gStyle->SetPadColor(10);
    gStyle->SetCanvasColor(10);
    gStyle->SetStatColor(10);

    TGaxis::SetMaxDigits(3);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);


  // create an instance of PndLmdPlotter the plotting helper class
  LumiFit::PndLmdPlotter lmd_plotter;

  PndLmdDataFacade lmd_data_facade;

  // create set of acceptances
  std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
      std::vector<PndLmdAcceptance> > acc_map;
  for (unsigned int i = 0; i < acceptance_files.size(); ++i) {
    TFile facc(acceptance_files[i].c_str(), "READ");
    std::vector<PndLmdAcceptance> accs = lmd_data_facade.getDataFromFile<
        PndLmdAcceptance>(facc);
    for (unsigned int j = 0; j < accs.size(); j++) {
      acc_map[std::make_pair(accs[j].getPrimaryDimension(),
          accs[j].getSecondaryDimension())].push_back(accs[j]);
    }
  }

  //std::cout << "we have " << acc_map.size() << " acceptances!"
  //    << std::endl;

  //NeatPlotting::PlotBundle bundle = lmd_plotter.createAcceptanceErrorPlot(
  //    acc_map.begin()->second);


  NeatPlotting::GraphAndHistogramHelper gh_helper;

  std::vector<NeatPlotting::GraphPoint> points;

  NeatPlotting::GraphPoint gp;

  for (auto const& acc : acc_map) {
    gp.x = acc.first.first.bins;
    gp.y = 100.0*meanSignedAverageError(acc.second.front());
    points.push_back(gp);

  }

  TGraph* graph = gh_helper.makeGraph(points);

  NeatPlotting::DataObjectStyle style;
  style.draw_option = "PE";
  style.marker_style.marker_style = 24;
  style.marker_style.marker_size = 1.4;

  NeatPlotting::PlotBundle bundle;
  bundle.addGraph(graph, style);
  //style.marker_style.marker_style = 23;
  //style.marker_style.marker_color = 9;
  //bundle.addGraph(graph_50, style);

  NeatPlotting::PlotStyle plot_style;
  plot_style.y_axis_style.axis_title_text_offset = 1.22;
  //plot_style.palette_color_style = 1;

  bundle.plot_axis.x_axis_title = "#theta_{x} & #theta_{y} binning";
  bundle.plot_axis.y_axis_title = "mean acceptance error /%";

  TCanvas c;

  bundle.drawOnCurrentPad(plot_style);

  c.SaveAs("acc_mean_errors_vs_binning.pdf");
}

int main(int argc, char* argv[]) {
  if (argc >= 2) {
    std::vector<std::string> acceptance_file_urls;
    for (unsigned int i = 1; i < argc; ++i)
      acceptance_file_urls.push_back(std::string(argv[i]));
    analyzeAcceptances(acceptance_file_urls);
    return 0;
  }
  return 1;
}
