#include "ui/PndLmdPlotter.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"
#include "ui/PndLmdDataFacade.h"

#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <sstream>

#include "boost/filesystem.hpp"
#include "boost/regex.hpp"

#include "TGaxis.h"
#include "TFitResult.h"
#include "TF2.h"

double gaus2d(double *x, double *par) {
  return par[0]
      * std::exp(
          -0.5 / (1 - std::pow(par[5], 2))
              * (std::pow(x[0] - par[1], 2) / std::pow(par[2], 2)
                  + std::pow(x[1] - par[3], 2) / std::pow(par[4], 2)
                  - 2 * par[5] * (x[0] - par[1]) * (x[1] - par[3])
                      / (par[2] * par[4])));
}

double resfunc(double *x, double *par) {
  return par[0] * std::exp(x[0] * par[1]) + par[2];
}

void makeResolutionPlots(std::vector<std::string> paths) {
  std::cout << "Generating lumi plots for fit results....\n";

// A small helper class that helps to construct lmd data objects
  PndLmdDataFacade lmd_data_facade;

  LumiFit::PndLmdPlotter lmd_plotter;

// ================================ BEGIN CONFIG ================================ //
// PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
// you can fine tune it and overwrite the default values
  gStyle->SetPadRightMargin(0.17);
  gStyle->SetPadLeftMargin(0.124);
  gStyle->SetPadBottomMargin(0.124);
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadColor(10);
  gStyle->SetCanvasColor(10);
  gStyle->SetStatColor(10);

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

// ================================= END CONFIG ================================= //

// create and fill data map first of all
  std::vector<PndLmdHistogramData> data_vec;

  std::vector<std::string> file_paths;
  // ------ get files -------------------------------------------------------
  for (auto const path : paths) {
    std::vector<std::string> temp_file_paths = lmd_data_facade.findFilesByName(
        path, "merge_data", "lmd_res_hist_data_.*.root");
    file_paths.insert(file_paths.end(), temp_file_paths.begin(),
        temp_file_paths.end());
  }

  for (unsigned int j = 0; j < file_paths.size(); j++) {
    std::string fullpath = file_paths[j];
    TFile fdata(fullpath.c_str(), "READ");

    // read in data from a root file which will return a map of PndLmdAngularData objects
    std::vector<PndLmdHistogramData> temp_data_vec =
        lmd_data_facade.getDataFromFile<PndLmdHistogramData>(fdata);

    // append all data objects to the end of the corresponding data map vectors
    data_vec.insert(data_vec.end(), temp_data_vec.begin(), temp_data_vec.end());
  }

  // =============================== BEGIN PLOTTING =============================== //

  std::stringstream basepath;
  basepath << std::getenv("HOME") << "/plots";

  std::stringstream filepath_base;
  filepath_base << basepath.str() << "/resolution";

  boost::filesystem::create_directories(filepath_base.str());

  double plot_range_low = 0.0015;
  double plot_range_high = 0.01;

  std::stringstream filepath;
  TCanvas c("", "", 1000, 880);

  NeatPlotting::PlotStyle single_plot_style;
  single_plot_style.x_axis_style.axis_text_style.text_size = 0.05;
  single_plot_style.y_axis_style.axis_text_style.text_size = 0.05;

  TGraphErrors *res_x_vs_plab = new TGraphErrors(data_vec.size());
  TGraphErrors *res_y_vs_plab = new TGraphErrors(data_vec.size());

  // reco single plot
  unsigned int counter(0);
  for (auto const resolution : data_vec) {
    //plot resolution first
    NeatPlotting::PlotBundle res2d_plot_bundle =
        lmd_plotter.makeResolutionGraphBundle(resolution);
    //res2d_plot_bundle.plot_axis.x_axis_range.low = plot_range_low;
    //res2d_plot_bundle.plot_axis.x_axis_range.high = plot_range_high;
    //res2d_plot_bundle.plot_axis.x_axis_range.active = true;
    res2d_plot_bundle.plot_decoration.draw_labels = false;
    res2d_plot_bundle.drawOnCurrentPad(single_plot_style);
    filepath.str("");
    filepath << filepath_base.str() << "/resolution2d_"
        << resolution.getLabMomentum() << "GeV.png";
    c.SaveAs(filepath.str().c_str());

    // now fit with a 2d gaus and get resolution
    TH2D* hist = resolution.get2DHistogram();
    TF2* f2 = new TF2("f2", gaus2d, -0.001, 0.001, -0.001, 0.001, 6);
    f2->SetParameters(300, hist->GetMean(1), hist->GetRMS(1), hist->GetMean(2),
        hist->GetRMS(2), hist->GetSkewness());
    //f2->SetParLimits(0, 1e6, 1e9);
    f2->SetParLimits(2, 0.0, 1.0);
    f2->SetParLimits(4, 0.0, 1.0);

    TFitResult *fit_result = hist->Fit(f2, "LSV").Get();

    res_x_vs_plab->SetPoint(counter, resolution.getLabMomentum(),
        1000.0 * fit_result->GetParams()[2]);
    res_x_vs_plab->SetPointError(counter, 0.0,
        1000.0 * fit_result->GetErrors()[2]);

    res_y_vs_plab->SetPoint(counter, resolution.getLabMomentum(),
        1000.0 * fit_result->GetParams()[4]);
    res_y_vs_plab->SetPointError(counter, 0.0,
        1000.0 * fit_result->GetErrors()[4]);

    ++counter;
  }

  /*gStyle->SetPadRightMargin(0.03);
   gStyle->SetPadLeftMargin(0.122);
   gStyle->SetPadBottomMargin(0.125);
   gStyle->SetPadTopMargin(0.03);*/

  c.Size(1000, 700);
  c.SetRightMargin(0.03);
  c.SetLeftMargin(0.122);
  c.SetBottomMargin(0.125);
  c.SetTopMargin(0.03);

  gPad->Update();

  TF1* f1 = new TF1("f11", resfunc, 1.5, 15, 3);
  f1->SetLineColor(1);
  f1->SetParameters(1.0, -1.0, 0.1);
  res_x_vs_plab->Fit(f1, "LRV+");
  TF1* f12 = new TF1("f12", resfunc, 1.5, 15, 3);
  f12->SetLineColor(8);
  f12->SetParameters(1.0, -1.0, 0.1);
  res_y_vs_plab->Fit(f12, "LRV+");

  //plot resolution first
  NeatPlotting::PlotBundle plot_bundle;
  NeatPlotting::DataObjectStyle objstyle;
  objstyle.marker_style.marker_size = 1.5;
  objstyle.draw_option = "AP";
  plot_bundle.addGraph(res_x_vs_plab, objstyle);
  objstyle.marker_style.marker_color = 8;
  plot_bundle.addGraph(res_y_vs_plab, objstyle);

  plot_bundle.plot_axis.x_axis_title = "p_{lab} /GeV/c";
  plot_bundle.plot_axis.y_axis_title = "Resolution #theta /mrad";

  /*
   1  p0           8.24570e-01   2.28543e-04
   2  p1          -4.76194e-01   1.04945e-04
   3  p2           9.70981e-02   8.51661e-06

   1  p0           5.30809e-01   1.33039e-04
   2  p1          -5.40334e-01   1.29035e-04
   3  p2           9.33062e-02   7.02693e-06
   */

  NeatPlotting::TextStyle text_style;
  text_style.text_color = 1;
  NeatPlotting::PlotLabel label1("major radius:", text_style);
  NeatPlotting::PlotLabel label2("0.097 + 0.825 exp(-0.476 p_{lab})", text_style);
  label1.setRelativePosition(0.45, 0.95);
  label2.setRelativePosition(0.45, 0.89);
  plot_bundle.plot_decoration.labels.push_back(label1);
  plot_bundle.plot_decoration.labels.push_back(label2);

  text_style.text_color = 8;
  NeatPlotting::PlotLabel label3("minor radius:", text_style);
  NeatPlotting::PlotLabel label4("0.093 + 0.531 exp(-0.540 p_{lab})", text_style);
  label3.setRelativePosition(0.45, 0.83);
  label4.setRelativePosition(0.45, 0.77);
  plot_bundle.plot_decoration.labels.push_back(label3);
  plot_bundle.plot_decoration.labels.push_back(label4);

  //plot_bundle.plot_decoration.label_text_leftpos = 7;
  //plot_bundle.plot_decoration.label_text_toppos = 0.52;
  //plot_bundle.plot_decoration.label_text_spacing = 0.07;

  plot_bundle.plot_decoration.draw_labels = true;
  plot_bundle.drawOnCurrentPad(single_plot_style);
  filepath.str("");
  filepath << filepath_base.str() << "/res_vs_plab.pdf";
  c.SaveAs(filepath.str().c_str());
  // ================================ END PLOTTING ================================ //
}

void displayInfo() {
// display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "paths to resolution data" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    std::vector<std::string> paths;
    for (int i = 1; i < argc; ++i)
      paths.push_back(argv[i]);
    makeResolutionPlots(paths);
  } else
    displayInfo();

  return 0;
}

