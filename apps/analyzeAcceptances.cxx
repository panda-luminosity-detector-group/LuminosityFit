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

TH1D* createBinContentDistributionHistogram(const PndLmdAcceptance& acc) {
  TH1D* blub = new TH1D("", "", 101, -0.5, 1000.5);
  std::cout << "calculating mean signed error for acceptance...\n";
  TEfficiency *eff = acc.getAcceptance2D(); // false = angular acceptance
  TCanvas c;
  eff->Draw("colz");
  c.Update();

  TH2D* acchist2d = (TH2D*) eff->GetCopyPassedHisto();

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
      int bin = acchist2d->FindFixBin(
          acchist2d->GetXaxis()->GetXmin() + (0.5 + ix) * dx,
          acchist2d->GetYaxis()->GetXmin() + (0.5 + iy) * dy);
      blub->Fill(acchist2d->GetBinContent(bin));
    }
  }

  return blub;
}

unsigned int getEmptyNeighbourBins(const PndLmdAcceptance& acc) {
  unsigned int empty_neighbouring_bins(0);

  std::cout << "calculating mean signed error for acceptance...\n";
  TEfficiency *eff = acc.getAcceptance2D(); // false = angular acceptance
  TCanvas c;
  eff->Draw("colz");
  c.Update();

  TH2D* acchist2d = (TH2D*) eff->GetCopyPassedHisto();

  double dx = (acchist2d->GetXaxis()->GetXmax()
      - acchist2d->GetXaxis()->GetXmin()) / acchist2d->GetXaxis()->GetNbins();
  double dy = (acchist2d->GetYaxis()->GetXmax()
      - acchist2d->GetYaxis()->GetXmin()) / acchist2d->GetYaxis()->GetNbins();

  for (unsigned int ix = 2; ix < acchist2d->GetXaxis()->GetNbins(); ++ix) {
    for (unsigned int iy = 2; iy < acchist2d->GetYaxis()->GetNbins(); ++iy) {
      unsigned int counter(0);
      int bin = acchist2d->FindFixBin(
          acchist2d->GetXaxis()->GetXmin() + (0.5 + ix) * dx,
          acchist2d->GetYaxis()->GetXmin() + (0.5 + iy) * dy);
      if (acchist2d->GetBinContent(bin) == 0) {
        for (int iix = -1; iix < 2; ++iix) {
          for (int iiy = -1; iiy < 2; ++iiy) {
            int tempbin = acchist2d->FindFixBin(
                acchist2d->GetXaxis()->GetXmin() + (0.5 + ix + iix) * dx,
                acchist2d->GetYaxis()->GetXmin() + (0.5 + iy + iiy) * dy);
            if (acchist2d->GetBinContent(tempbin) > 0) {
              ++counter;
            }
          }
        }
        if(counter > 7)
          ++empty_neighbouring_bins;
      }
    }
  }

  return empty_neighbouring_bins;
}

unsigned int getEmptyNeighbourBins(const PndLmdAngularData& data) {
  unsigned int empty_neighbouring_bins(0);

  TH2D* acchist2d = data.get2DHistogram();

  double dx = (acchist2d->GetXaxis()->GetXmax()
      - acchist2d->GetXaxis()->GetXmin()) / acchist2d->GetXaxis()->GetNbins();
  double dy = (acchist2d->GetYaxis()->GetXmax()
      - acchist2d->GetYaxis()->GetXmin()) / acchist2d->GetYaxis()->GetNbins();

  for (unsigned int ix = 2; ix < acchist2d->GetXaxis()->GetNbins(); ++ix) {
    for (unsigned int iy = 2; iy < acchist2d->GetYaxis()->GetNbins(); ++iy) {
      unsigned int counter(0);
      int bin = acchist2d->FindFixBin(
          acchist2d->GetXaxis()->GetXmin() + (0.5 + ix) * dx,
          acchist2d->GetYaxis()->GetXmin() + (0.5 + iy) * dy);
      if (acchist2d->GetBinContent(bin) == 0) {
        for (int iix = -1; iix < 2; ++iix) {
          for (int iiy = -1; iiy < 2; ++iiy) {
            int tempbin = acchist2d->FindFixBin(
                acchist2d->GetXaxis()->GetXmin() + (0.5 + ix + iix) * dx,
                acchist2d->GetYaxis()->GetXmin() + (0.5 + iy + iiy) * dy);
            if (acchist2d->GetBinContent(tempbin) > 0) {
              ++counter;
            }
          }
        }
        if(counter > 7)
          ++empty_neighbouring_bins;
      }
    }
  }

  return empty_neighbouring_bins;
}

void analyzeAcceptances(std::vector<std::string> &acceptance_files) {
  std::cout << "Generating acceptance comparison plots ....\n";

  // ================================ BEGIN CONFIG ================================ //
  // PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
  // you can fine tune it and overwrite the default values
  gStyle->SetPadRightMargin(0.07);
  gStyle->SetPadLeftMargin(0.125);
  gStyle->SetPadBottomMargin(0.126);
  gStyle->SetPadTopMargin(0.07);
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
  /*std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
      std::vector<PndLmdAcceptance> > acc_map;
  for (unsigned int i = 0; i < acceptance_files.size(); ++i) {
    TFile facc(acceptance_files[i].c_str(), "READ");
    std::vector<PndLmdAcceptance> accs = lmd_data_facade.getDataFromFile<
        PndLmdAcceptance>(facc);
    for (unsigned int j = 0; j < accs.size(); j++) {
      acc_map[std::make_pair(accs[j].getPrimaryDimension(),
          accs[j].getSecondaryDimension())].push_back(accs[j]);
    }
  }*/

  std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
        std::vector<PndLmdAngularData> > data_map;
    for (unsigned int i = 0; i < acceptance_files.size(); ++i) {
      TFile facc(acceptance_files[i].c_str(), "READ");
      std::vector<PndLmdAngularData> datavec = lmd_data_facade.getDataFromFile<
          PndLmdAngularData>(facc);

       LumiFit::LmdDimensionOptions lmd_dim_opt;
       lmd_dim_opt.dimension_type = LumiFit::THETA_X;
       lmd_dim_opt.track_type = LumiFit::MC_ACC;

       LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
       datavec = lmd_data_facade.filterData<PndLmdAngularData>(
           datavec, filter);

      for (unsigned int j = 0; j < datavec.size(); j++) {
        data_map[std::make_pair(datavec[j].getPrimaryDimension(),
            datavec[j].getSecondaryDimension())].push_back(datavec[j]);
      }
    }

  //std::cout << "we have " << acc_map.size() << " acceptances!"
  //    << std::endl;

  //NeatPlotting::PlotBundle bundle = lmd_plotter.createAcceptanceErrorPlot(
  //    acc_map.begin()->second);

  NeatPlotting::DataObjectStyle style;
  style.draw_option = "";
  style.marker_style.marker_style = 24;
  style.marker_style.marker_size = 1.4;

  NeatPlotting::PlotStyle plot_style;
  plot_style.y_axis_style.log_scale = true;
  plot_style.y_axis_style.axis_title_text_offset = 1.22;
  //plot_style.palette_color_style = 1;

  NeatPlotting::GraphAndHistogramHelper gh_helper;
  std::vector<NeatPlotting::GraphPoint> points;

  NeatPlotting::GraphPoint gp;

 // NeatPlotting::Booky booky;

  std::vector<NeatPlotting::GraphPoint> acc_check_points;

  for (auto const& ang_data : data_map) {
    gp.x = ang_data.first.first.bins;
    //gp.y = 100.0 * meanSignedAverageError(ang_data.second.front());
   // points.push_back(gp);

  //  unsigned int counter(gp.x / 50 - 1);
  //  NeatPlotting::PlotBundle bundle2;
   // TH1D* temphist = createBinContentDistributionHistogram(acc.second.front());
   // bundle2.addHistogram(temphist, style);
   // booky.addPlotToCurrentBookyPage(bundle2, plot_style,
   //     std::make_pair(counter / 4 + 1, (counter - 1) % 4 + 1));

    /*gp.y = temphist->GetBinContent(1);
     for(unsigned int i=1; i <= 20; ++i) {
     gp.y += temphist->GetBinContent(i);
     }*/
    gp.y = getEmptyNeighbourBins(ang_data.second.front());///(gp.x*gp.x);
    acc_check_points.push_back(gp);
  }
//  booky.addCurrentPageToBooky();

  TGraph* graph = gh_helper.makeGraph(points);

  plot_style.y_axis_style.log_scale = false;
  style.draw_option = "AP";
  NeatPlotting::PlotBundle bundle;
  bundle.addGraph(graph, style);
  //style.marker_style.marker_style = 23;
  //style.marker_style.marker_color = 9;
  //bundle.addGraph(graph_50, style);

  bundle.plot_axis.x_axis_title = "#theta_{x} & #theta_{y} binning";
  bundle.plot_axis.y_axis_title = "mean acceptance error /%";

  TCanvas c;

  bundle.drawOnCurrentPad(plot_style);
  c.SaveAs("acc_mean_errors_vs_binning.pdf");

  NeatPlotting::PlotBundle bundle3;
  bundle3.plot_axis.x_axis_title = "#theta_{x} & #theta_{y} binning";
  bundle3.plot_axis.y_axis_title = "# of isolated empty bins";
  bundle3.addGraph(gh_helper.makeGraph(acc_check_points), style);
  bundle3.drawOnCurrentPad(plot_style);
  c.SaveAs("acc_stats_check.pdf");

 // booky.createBooky("booky.pdf");
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
