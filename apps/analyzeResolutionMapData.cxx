#include "ui/PndLmdDataFacade.h"
#include "ui/PndLmdPlotter.h"
#include "data/PndLmdMapData.h"

#include <iostream>               // for std::cout
#include <utility>
#include <vector>
#include <sstream>
#include <cmath>

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TGaxis.h"

struct Stats {
  double mean_relative_precision;
  unsigned int mean_entries;
  unsigned int mean_num_contributors;

  TH2D* relative_error_distribution;
  TH2D* entries_distribution_2d;
  TH2D* total_distribution_2d;
  TH1D* entries_distribution;
  TH1D* total_distribution;

  Stats() :
      mean_relative_precision(0.0), mean_entries(0), mean_num_contributors(0) {

  }
};

Stats calcStats(const PndLmdMapData& res_map) {
  std::cout << "calculating statistics for resolution map...\n";

  Stats st;
  st.relative_error_distribution = new TH2D("asdf", "",
      res_map.getPrimaryDimension().bins, -0.013, 0.013,
      res_map.getPrimaryDimension().bins, -0.013, 0.013);
  st.entries_distribution = new TH1D("", "", 200, 0, 200);
  st.total_distribution = new TH1D("", "", 2000, 0, 500);
  st.entries_distribution_2d = new TH2D("asdf", "",
      res_map.getPrimaryDimension().bins, -0.013, 0.013,
      res_map.getPrimaryDimension().bins, -0.013, 0.013);
  st.total_distribution_2d = new TH2D("asdf", "",
      res_map.getPrimaryDimension().bins, -0.013, 0.013,
      res_map.getPrimaryDimension().bins, -0.013, 0.013);

  unsigned int counter(0);

  const std::map<Point2D, Point2DCloud>& hit_map = res_map.getHitMap();
  for (auto const& mc_bin : hit_map) {
    double mean_rel_error(0.0);
    unsigned int tot(mc_bin.second.total_count);
    unsigned int entries_sum(0);

    for (auto const& reco_bin : mc_bin.second.points) {
      unsigned int n(reco_bin.second);
      entries_sum += n;
      double ratio(1.0*n/tot);
      mean_rel_error += std::sqrt(1.0  - ratio) * ratio;
      st.entries_distribution->Fill(n);
    }
    st.mean_entries += entries_sum;
    st.mean_relative_precision += mean_rel_error;
    counter = counter + mc_bin.second.points.size();
    st.mean_num_contributors += mc_bin.second.points.size();

    st.total_distribution->Fill(tot);
    st.total_distribution_2d->Fill(mc_bin.first.x, mc_bin.first.y, tot);
    st.entries_distribution_2d->Fill(mc_bin.first.x, mc_bin.first.y,
        entries_sum / mc_bin.second.points.size());
    st.relative_error_distribution->Fill(mc_bin.first.x, mc_bin.first.y,
        mean_rel_error / mc_bin.second.points.size());
  }
  st.mean_num_contributors /= hit_map.size();
  st.mean_entries /= counter;
  st.mean_relative_precision /= counter;
  return st;
}

std::vector<NeatPlotting::GraphPoint> generateGraphs(
    const std::map<std::string, std::map<unsigned int, double> >& data) {
  NeatPlotting::DataObjectStyle style;
  style.draw_option = "";
  style.marker_style.marker_style = 24;
  style.marker_style.marker_size = 1.4;

  NeatPlotting::PlotStyle plot_style;
  plot_style.y_axis_style.log_scale = true;
  plot_style.y_axis_style.axis_title_text_offset = 1.22;
  //plot_style.palette_color_style = 1;

  NeatPlotting::GraphAndHistogramHelper gh_helper;

  NeatPlotting::GraphPoint gp;

  for (auto const& graph_data : data) {
    std::vector<NeatPlotting::GraphPoint> gps;
    for (auto const& point : graph_data.second) {
      gp.x = point.first;
      gp.y = point.second;
      gps.push_back(gp);
    }

    TGraph* graph = gh_helper.makeGraph(gps);

    plot_style.y_axis_style.log_scale = false;
    style.draw_option = "AP";
    NeatPlotting::PlotBundle bundle;
    bundle.addGraph(graph, style);
    //style.marker_style.marker_style = 23;
    //style.marker_style.marker_color = 9;
    //bundle.addGraph(graph_50, style);

    bundle.plot_axis.x_axis_title = "#theta_{x} & #theta_{y} binning";
    bundle.plot_axis.y_axis_title = graph_data.first;

    TCanvas c;

    bundle.drawOnCurrentPad(plot_style);
    std::stringstream ss;
    ss << graph_data.first << ".pdf";
    c.SaveAs(ss.str().c_str());
  }
}

void plotHist(TH2D* hist, std::string outfilename) {
  NeatPlotting::DataObjectStyle style;
  style.draw_option = "";
  style.marker_style.marker_style = 24;
  style.marker_style.marker_size = 1.4;

  NeatPlotting::PlotStyle plot_style;
  plot_style.y_axis_style.log_scale = false;
  plot_style.y_axis_style.axis_title_text_offset = 1.22;

  plot_style.z_axis_style.log_scale = true;
  style.draw_option = "COL";
  NeatPlotting::PlotBundle bundle;
  bundle.addHistogram(hist, style);
  //style.marker_style.marker_style = 23;
  //style.marker_style.marker_color = 9;
  //bundle.addGraph(graph_50, style);

  bundle.plot_axis.x_axis_title = "#theta_{x} & #theta_{y} binning";
  bundle.plot_axis.y_axis_title = "";

  TCanvas c;

  bundle.drawOnCurrentPad(plot_style);

  c.SaveAs(outfilename.c_str());
}

void analyzeResolutionMaps(std::vector<std::string> &res_map_files) {
  std::cout << "Running resolution map analyze script ....\n";

  // ================================ BEGIN CONFIG ================================ //
  // PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
  // you can fine tune it and overwrite the default values
  gStyle->SetPadRightMargin(0.125);
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

  std::map<std::pair<LumiFit::LmdDimension, LumiFit::LmdDimension>,
      std::vector<PndLmdMapData> > data_map;
  for (auto const& res_map_file : res_map_files) {
    TFile f(res_map_file.c_str(), "READ");
    std::vector<PndLmdMapData> datavec = lmd_data_facade.getDataFromFile<
        PndLmdMapData>(f);

    /* LumiFit::LmdDimensionOptions lmd_dim_opt;
     lmd_dim_opt.dimension_type = LumiFit::THETA_X;
     lmd_dim_opt.track_type = LumiFit::MC_ACC;

     LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter filter(lmd_dim_opt);
     datavec = lmd_data_facade.filterData<PndLmdAngularData>(
     datavec, filter);*/

    for (auto const& data : datavec) {
      data_map[std::make_pair(data.getPrimaryDimension(),
          data.getSecondaryDimension())].push_back(data);
    }
  }

  std::map<std::string, std::map<unsigned int, double> > stats_data;

  for (auto const& res_data : data_map) {
    Stats s = calcStats(res_data.second[0]);
    stats_data["mean_relative_error"][res_data.first.first.bins] =
        s.mean_relative_precision;
    stats_data["mean_entries"][res_data.first.first.bins] = s.mean_entries;
    stats_data["mean_contributors"][res_data.first.first.bins] =
        s.mean_num_contributors;

    std::stringstream ss;
    ss << "res_errors_" << res_data.first.first.bins << "_log.pdf";
    plotHist(s.relative_error_distribution, ss.str());

    ss.str("");
    ss << "entries_dist_" << res_data.first.first.bins << "_log.pdf";
    plotHist(s.entries_distribution_2d, ss.str());

    ss.str("");
    ss << "totals_dist_" << res_data.first.first.bins << "_log.pdf";
    plotHist(s.total_distribution_2d, ss.str());

  }
  generateGraphs(stats_data);

}

int main(int argc, char* argv[]) {
  if (argc >= 2) {
    std::vector<std::string> resmap_file_urls;
    for (unsigned int i = 1; i < argc; ++i)
      resmap_file_urls.push_back(std::string(argv[i]));
    analyzeResolutionMaps(resmap_file_urls);
    return 0;
  }
  return 1;
}
