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
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include "eigen3/Eigen/Core"
#include "eigen3/Eigen/LU"
#include "eigen3/Eigen/SVD"
//#include "eigen3/Eigen/Array"
#include "eigen3/Eigen/src/Geometry/Umeyama.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

struct TrafoMatrixOptimizeFunction {
  Eigen::MatrixXd values;
  Eigen::MatrixXd ref_values;

  TrafoMatrixOptimizeFunction(const Eigen::MatrixXd& values_,
      const Eigen::MatrixXd& ref_values_) :
      values(values_), ref_values(ref_values_) {
    values.conservativeResize(3, Eigen::NoChange);
    ref_values.conservativeResize(3, Eigen::NoChange);

    for (unsigned int i = 0; i < values.cols(); ++i) {
      values(2, i) = 1.0;
      ref_values(2, i) = 1.0;
    }
  }

  double evaluate(const double *x) {
    auto temp_mat = values;
    auto temp = temp_mat.array();
    temp.row(0) = temp.row(0) + x[4];
    temp.row(1) = temp.row(1) + x[5];

    Eigen::Matrix3d trafo;
    trafo(0, 0) = x[0];
    trafo(0, 1) = x[1];
    trafo(0, 2) = 0.0;
    trafo(1, 0) = x[2];
    trafo(1, 1) = x[3];
    trafo(1, 2) = 0.0;
    trafo(2, 0) = 0.0;
    trafo(2, 1) = 0.0;
    trafo(2, 2) = 1.0;

    double result = ((trafo * temp.matrix()) - ref_values).block(0, 0, 2,
        temp.cols()).squaredNorm() / temp.cols();
    return result;
  }
};

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
        if (counter > 7)
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
        if (counter > 7)
          ++empty_neighbouring_bins;
      }
    }
  }

  return empty_neighbouring_bins;
}

void determineOffsetParameters(
    const std::map<std::pair<double, double>, std::vector<PndLmdAcceptance> >& accs) {
  std::cout << "calculating offset parameters based on the acceptances...\n";

  std::map<std::pair<double, double>, std::pair<double, double> > data;

  for (auto const& acc : accs) {
    TEfficiency *eff = acc.second[0].getAcceptance2D(); // false = angular acceptance
    TCanvas c;
    eff->Draw("colz");
    c.Update();

    TH2D* acchist2d = (TH2D*) eff->GetCopyPassedHisto();

    //get mean x and y and compare values with others
    TH1D* projx = acchist2d->ProjectionX("xproj");
    TH1D* projy = acchist2d->ProjectionY("yproj");

    data[acc.first] = std::make_pair(1000.0 * projx->GetMean(),
        1000.0 * projy->GetMean());
  }

  for (auto ele : data) {
    std::cout << ele.first.first << ":" << ele.first.second << " -> "
        << ele.second.first << ":" << ele.second.second << std::endl;
    std::cout << "diff: " << ele.first.first + 1000.0 * ele.second.first
        << " : " << ele.first.second + 1000.0 * ele.second.second << std::endl;
    std::cout << "ratio: " << ele.first.first / (1000.0 * ele.second.first)
        << " : " << ele.first.second / (1000.0 * ele.second.second)
        << std::endl;
  }

  NeatPlotting::GraphAndHistogramHelper gh_helper;
  std::vector<NeatPlotting::GraphPoint> points;

  NeatPlotting::GraphPoint gp;

  // NeatPlotting::Booky booky;

  std::vector<NeatPlotting::GraphPoint> graph_points;
  std::vector<NeatPlotting::GraphPoint> graph_points_x;
  std::vector<NeatPlotting::GraphPoint> graph_points_y;

  for (auto const& data_point : data) {
    gp.x = data_point.first.first;
    gp.y = data_point.second.first;
    graph_points_x.push_back(gp);
    gp.x = data_point.second.first;
    gp.y = data_point.second.second;
    graph_points_y.push_back(gp);

    gp.x = data_point.second.first;
    gp.y = data_point.second.second;
    graph_points.push_back(gp);
  }

  TGraph* graph = gh_helper.makeGraph(graph_points_x);

  NeatPlotting::DataObjectStyle style;
  style.draw_option = "";
  style.marker_style.marker_style = 24;
  style.marker_style.marker_size = 1.4;

  NeatPlotting::PlotStyle plot_style;
  plot_style.y_axis_style.log_scale = true;
  plot_style.y_axis_style.axis_title_text_offset = 1.22;
  plot_style.y_axis_style.log_scale = false;
  style.draw_option = "AP";

  NeatPlotting::PlotBundle bundle;
  bundle.addGraph(graph, style);
  //style.marker_style.marker_style = 23;
  //style.marker_style.marker_color = 9;
  //bundle.addGraph(graph_50, style);

  bundle.plot_axis.x_axis_title = "sim. offset x";
  bundle.plot_axis.y_axis_title = "acc. mean offset x";

  TCanvas c;

  bundle.drawOnCurrentPad(plot_style);
  c.SaveAs("acc_offset_correlations_x.pdf");

  graph = gh_helper.makeGraph(graph_points_y);

  NeatPlotting::PlotBundle bundle2;
  bundle2.addGraph(graph, style);
  //style.marker_style.marker_style = 23;
  //style.marker_style.marker_color = 9;
  //bundle.addGraph(graph_50, style);

  bundle2.plot_axis.x_axis_title = "sim. offset y";
  bundle2.plot_axis.y_axis_title = "acc. mean offset y";

  bundle2.drawOnCurrentPad(plot_style);
  c.SaveAs("acc_offset_correlations_y.pdf");

  // determine transformation matrix between point clouds
  Eigen::MatrixXd angular_offset_values(2, data.size()), offsets(2,
      data.size());
  unsigned int counter(0);
  for (auto element : data) {
    offsets(0, counter) = element.first.first;
    offsets(1, counter) = element.first.second;

    angular_offset_values(0, counter) = element.second.first;
    angular_offset_values(1, counter) = element.second.second;

    ++counter;
  }

  auto trafo_matrix = Eigen::umeyama(offsets, angular_offset_values, true);
  std::cout << trafo_matrix << std::endl;

  // Choose method upon creation between:
  // kMigrad, kSimplex, kCombined,
  // kScan, kFumili
  ROOT::Math::Minimizer* min = ROOT::Math::Factory::CreateMinimizer("Minuit2",
      "Migrad");

  min->SetMaxFunctionCalls(1000000);
  min->SetMaxIterations(100000);
  min->SetTolerance(0.0001);
  min->SetPrintLevel(5);

  TrafoMatrixOptimizeFunction optfunc(offsets, angular_offset_values);

  ROOT::Math::Functor f(&optfunc, &TrafoMatrixOptimizeFunction::evaluate, 6);
  double variable[8] = { -0.460733, -0.49298, 0.49298, -0.460733, 0.0224446,
      -0.0103477, 0.01, 0.01 };

  min->SetFunction(f);

  // Set the free variables to be minimized!
  min->SetVariable(0, "x11", variable[0], std::fabs(0.1 * variable[0]));
  min->SetVariable(1, "x12", variable[1], std::fabs(0.1 * variable[1]));
  min->SetVariable(2, "x21", variable[2], std::fabs(0.1 * variable[2]));
  min->SetVariable(3, "x22", variable[3], std::fabs(0.1 * variable[3]));
  min->SetVariable(4, "t1", variable[4], std::fabs(0.1 * variable[4]));
  min->SetVariable(5, "t2", variable[5], std::fabs(0.1 * variable[5]));
  //min->SetVariable(6, "at1", variable[6], step[6]);
  //min->SetVariable(7, "at2", variable[7], step[7]);

  min->Minimize();

  const double *xs = min->X();
  std::cout << "Optimal matrix: \n" << xs[0] << " " << xs[1] << " " << xs[4]
      << std::endl << xs[2] << " " << xs[3] << " " << xs[5] << std::endl;

  auto temp_mat = offsets;
  offsets.conservativeResize(3, Eigen::NoChange);
  for (unsigned int i = 0; i < offsets.cols(); ++i) {
    offsets(2, i) = 1.0;
  }

  auto new_vals = trafo_matrix * offsets;

  auto temp = temp_mat.array();
  temp.row(0) = temp.row(0) + xs[4];
  temp.row(1) = temp.row(1) + xs[5];

  Eigen::Matrix2d tm;
  tm(0, 0) = xs[0];
  tm(0, 1) = xs[1];
  tm(1, 0) = xs[2];
  tm(1, 1) = xs[3];
  std::cout << tm << std::endl;
  std::cout << temp.matrix() << std::endl;
  auto new_vals_mine = tm * temp.matrix();

  boost::property_tree::ptree pt;
  boost::property_tree::ptree translation_pt;
  translation_pt.put("t1", xs[4]);
  translation_pt.put("t2", xs[5]);
  pt.add_child("before_translation", translation_pt);
  boost::property_tree::ptree matrix_pt;
  matrix_pt.put("m11", xs[0]);
  matrix_pt.put("m12", xs[1]);
  matrix_pt.put("m21", xs[2]);
  matrix_pt.put("m22", xs[3]);
  pt.add_child("matrix", matrix_pt);
  write_json("offset_trafo_matrix.json", pt);

  std::vector<NeatPlotting::GraphPoint> graph_points_transformed;
  std::vector<NeatPlotting::GraphPoint> graph_points_transformed_mine;

  for (unsigned int i = 0; i < new_vals.cols(); ++i) {
    gp.x = new_vals(0, i);
    gp.y = new_vals(1, i);
    graph_points_transformed.push_back(gp);
    gp.x = new_vals_mine(0, i);
    gp.y = new_vals_mine(1, i);
    graph_points_transformed_mine.push_back(gp);
  }

  NeatPlotting::PlotBundle bundle22;
  TGraph* graph12 = gh_helper.makeGraph(graph_points);
  bundle22.addGraph(graph12, style);
  style.marker_style.marker_style = 23;
  style.marker_style.marker_color = 9;
  TGraph* graph22 = gh_helper.makeGraph(graph_points_transformed);
  bundle22.addGraph(graph22, style);
  style.marker_style.marker_style = 5;
  style.marker_style.marker_color = 8;
  TGraph* graph32 = gh_helper.makeGraph(graph_points_transformed_mine);
  bundle22.addGraph(graph32, style);

  bundle22.plot_axis.x_axis_title = "offset x";
  bundle22.plot_axis.y_axis_title = "offset y";

  bundle22.drawOnCurrentPad(plot_style);
  c.SaveAs("offset_transformed.pdf");
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

  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();

  std::map<std::pair<double, double>, std::vector<PndLmdAcceptance> > acc_map;
  for (unsigned int i = 0; i < acceptance_files.size(); ++i) {
    TFile facc(acceptance_files[i].c_str(), "READ");
    boost::filesystem::path pathname(acceptance_files[i]);

    std::vector<PndLmdAcceptance> datavec = lmd_data_facade.getDataFromFile<
        PndLmdAcceptance>(facc);

    lmd_runtime_config.readSimulationParameters(
        pathname.parent_path().string() + "/../../../../sim_params.config");
    auto xy_mean = std::make_pair(
        PndLmdRuntimeConfiguration::Instance().getSimulationParameters().get<
            double>("ip_mean_x"),
        PndLmdRuntimeConfiguration::Instance().getSimulationParameters().get<
            double>("ip_mean_y"));

    acc_map[xy_mean] = datavec;
  }
  determineOffsetParameters(acc_map);

  return;

  std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
      std::vector<PndLmdAngularData> > data_map;
  /* for (unsigned int i = 0; i < acceptance_files.size(); ++i) {
   TFile facc(acceptance_files[i].c_str(), "READ");

   std::vector<PndLmdAngularData> datavec = lmd_data_facade.getDataFromFile<
   PndLmdAngularData>(facc);

   LumiFit::LmdDimensionOptions lmd_dim_opt;
   lmd_dim_opt.dimension_type = LumiFit::THETA_X;
   lmd_dim_opt.track_type = LumiFit::MC_ACC;

   LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
   datavec = lmd_data_facade.filterData<PndLmdAngularData>(datavec, filter);

   for (unsigned int j = 0; j < datavec.size(); j++) {
   data_map[std::make_pair(datavec[j].getPrimaryDimension(),
   datavec[j].getSecondaryDimension())].push_back(datavec[j]);
   }
   }*/

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
    gp.y = getEmptyNeighbourBins(ang_data.second.front());   ///(gp.x*gp.x);
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
