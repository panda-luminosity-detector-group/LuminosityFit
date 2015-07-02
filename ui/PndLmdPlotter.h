#ifndef PNDLMDPLOTTER_H_
#define PNDLMDPLOTTER_H_

#include "ROOTPlotHelper.hpp"
#include "model/PndLmdModelFactory.h"
#include "fit/PndLmdFitOptions.h"
#include "PndLmdComparisonStructs.h"
#include "data/PndLmdFitDataBundle.h"

#ifndef __CINT__
#include "visualization/ROOT/ROOTPlotter.h"
#include "visualization/ModelVisualizationProperties1D.h"
#endif /* __CINT __ */

#include <map>

#include "TString.h"

class PndLmdAngularData;
class PndLmdAcceptance;
class ModelFitResult;
class PndLmdElasticDataBundle;

class TCanvas;

namespace LumiFit {

class PndLmdPlotter {
public:
	DataStructs::DimensionRange primary_dimension_plot_range;
	DataStructs::DimensionRange secondary_dimension_plot_range;

	struct LumiValues {
		NeatPlotting::GraphPoint position;
		double lumival;
		double lumierr;
		double lumiref;

		LumiValues(double x, double y, double lumival_, double lumierr_,
				double lumiref_) :
				lumival(lumival_), lumierr(lumierr_), lumiref(lumiref_) {
			position.x = x;
			position.y = y;
		}
	};

private:
	NeatPlotting::GraphAndHistogramHelper neat_plot_helper;

	PndLmdModelFactory model_factory;
	PndLmdFitDataBundle current_fit_bundle;

#ifndef __CINT__
	ROOTPlotter root_plotter;
#endif /* __CINT __ */

	/*std::pair<double, double> determinePlotRange(
	 std::map<TString, std::vector<LumiValues> > &result_map);
	 std::pair<double, double> determineDiffPlotRange(
	 std::map<TString, std::vector<combined_values> > &result_map);*/


	// TODO: this whole block of functions has to go somewhere else
	double getLuminosity(const ModelFitResult& model_fit_result) const;
	double getLuminositySysError(const ModelFitResult& model_fit_result) const;
	double getLuminosityStatError(const ModelFitResult& model_fit_result) const;
	double getLuminosityError(const ModelFitResult& model_fit_result) const;
	double getRedChiSquare(const ModelFitResult& model_fit_result) const;


	double calculateYPos(double line, double text_toppos, double text_spacing,
			bool log);

	std::vector<DataStructs::DimensionRange> generatePlotRange(
			const PndLmdAbstractData &lmd_abs_data,
			const EstimatorOptions &est_options) const;

	void applyPlotRanges(NeatPlotting::PlotBundle &plot_bundle) const;

#ifndef __CINT__
	std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D> create2DVisualizationProperties(
			const EstimatorOptions &est_options,
			const PndLmdAbstractData &lmd_abs_data) const;

	ModelVisualizationProperties1D create1DVisualizationProperties(
			const EstimatorOptions &est_options,
			const PndLmdAbstractData &lmd_abs_data) const;
#endif /* __CINT __ */

	bool compareLumiModelOptions(const boost::property_tree::ptree &pt,
			const LumiFit::LmdDimensionOptions &dim_opt) const;
	bool compareLumiModelOptions(const boost::property_tree::ptree &pt_lhs,
			const boost::property_tree::ptree &pt_rhs) const;

public:
	PndLmdPlotter();
	virtual ~PndLmdPlotter();

	std::string makeDirName(const PndLmdAbstractData &elastic_bundle) const;

	std::pair<double, double> calulateRelDiff(double value, double error,
			double ref) const;

	void setCurrentFitDataBundle(const PndLmdFitDataBundle &fit_data_bundle);

	TGraphAsymmErrors* createGraphFromFitResult(const PndLmdFitOptions &fit_opt,
			PndLmdAngularData &data) const;

	// create graphs or histograms representing the model
	TH2D* create2DHistogramFromFitResult(const PndLmdFitOptions &fit_opt,
			const PndLmdElasticDataBundle &elastic_data_bundle);

	TGraphAsymmErrors* createVertexGraphFromFitResult(
			const PndLmdFitOptions &fit_opt, const PndLmdHistogramData &data) const;

	/*TH2D* create2DHistogramFromFitResult(const PndLmdFitOptions &fit_opt,
			const PndLmdHistogramData &res_data) const;*/

	TGraphAsymmErrors* createSmearingGraphFromFitResult(
			const PndLmdFitOptions &fit_opt,
			const PndLmdHistogramData &res_data) const;

	/*std::pair<TGraphAsymmErrors*, TGraphAsymmErrors*> createResidual(
	 const PndLmdLumiFitOptions &fit_opt, PndLmdAngularData &data);*/

	TGraphAsymmErrors* createAcceptanceGraph(const PndLmdAcceptance *acc) const;

	TH2D* createAcceptanceHistogram(const PndLmdAcceptance& acc) const;

	TGraphAsymmErrors* generateDPMModelPartGraph(double plab,
			LumiFit::DPMElasticParts dpm_elastic_part,
			DataStructs::DimensionRange& plot_range);

	// the plot bundle creation functions ===========================================

	NeatPlotting::PlotBundle makeAcceptanceBundle1D(
			const PndLmdAcceptance& acc) const;

	NeatPlotting::PlotBundle makeAcceptanceBundle2D(
			const PndLmdAcceptance& acc) const;

	NeatPlotting::PlotBundle makeGraphBundle(PndLmdAngularData& data,
			const PndLmdFitOptions &fit_options, bool draw_data = true,
			bool draw_model = true, bool draw_label = true);

	NeatPlotting::PlotBundle makeResidualPlotBundle1D(PndLmdAngularData& data,
			const PndLmdFitOptions &fit_options);

	NeatPlotting::PlotBundle makeResolutionGraphBundle(
			const PndLmdHistogramData& res) const;

	/*NeatPlotting::PlotBundle2D makeResolutionGraphBundle2D(
	 PndLmdHistogramData& res);*/

	NeatPlotting::PlotBundle makeVertexGraphBundle1D(
			const PndLmdHistogramData& data) const;

	NeatPlotting::PlotBundle makeIPXYOverviewGraphBundle(
			const std::vector<PndLmdHistogramData> &vertex_data) const;

	NeatPlotting::PlotBundle makeXYOverviewHistogram(
			const std::vector<PndLmdElasticDataBundle> &elastic_data_bundles) const;

	NeatPlotting::PlotBundle makeTiltXYOverviewHistogram(
			const std::vector<PndLmdElasticDataBundle> &elastic_data_bundles,
			int draw_labels = 2) const;

	// the plot bundles creation functions ===========================================

	/*
	 std::pair<TGraphErrors*, TGraphErrors*> makeIPParameterDependencyGraphBundle(
	 std::vector<PndLmdVertexData> &vertex_data,
	 const LumiFit::LmdSimIPParameters &dependency, TString ytitle_subscript);


	 std::map<PndLmdLumiFitOptions, NeatPlotting::PlotBundle1D> makeGraphBundles1D(
	 std::vector<PndLmdAngularData> &data_vec,
	 LumiFit::PndLmdFitModelOptions &fitop_tmctruth);

	 std::map<PndLmdLumiFitOptions,
	 std::map<int, NeatPlotting::PlotBundle1D>,
	 LumiFit::Comparisons::fit_options_compare> makeGraphBundles1D(
	 std::vector<PndLmdAngularData> &data_vec);
	 */

	// systematics helper functions ===========================================
	NeatPlotting::PlotBundle createLowerFitRangeDependencyPlotBundle(
			const std::vector<PndLmdAngularData> &data_vec,
			const LmdDimensionOptions& dim_opt) const;

	std::map<PndLmdHistogramData, std::vector<PndLmdAngularData> > clusterIntoGroups(
			const std::vector<PndLmdAngularData> &data_vec) const;

	TGraphAsymmErrors* makeXYOverviewGraph(
			const std::vector<PndLmdHistogramData> &data_vec,
			double error_scaling_factor) const;

	TGraphAsymmErrors* createLowerFitRangeDependencyGraph(
			const std::vector<PndLmdAngularData> &prefiltered_data,
			const LmdDimensionOptions& dim_opt) const;

	NeatPlotting::SystematicsAnalyser::SystematicDependencyGraphBundle createLowerFitRangeDependencyGraphBundle(
			const std::vector<PndLmdAngularData> &prefiltered_data,
			const LmdDimensionOptions& dim_opt) const;

	// booky routines =======================================================

#ifndef __CINT__
	NeatPlotting::Booky makeLumiFitResultOverviewBooky(
			std::vector<PndLmdAngularData> &data_vec);

	void createAcceptanceComparisonBooky(
			std::vector<std::pair<PndLmdAcceptance, PndLmdAcceptance> > &acc_matches);

	NeatPlotting::Booky makeVertexFitResultBooky(
			const std::vector<PndLmdHistogramData> &data_vec);

	void makeVertexDifferencesBooky(std::vector<PndLmdHistogramData> &res_vec,
			std::vector<PndLmdHistogramData> &res_vec_ref);

	NeatPlotting::Booky makeResolutionFitResultBooky(
			const std::vector<PndLmdHistogramData> &data_vec, int x = 5,
			int y = 4) const;

#endif /* __CINT __ */

	NeatPlotting::PlotBundle createLumiFit2DPlotBundle(TH2D* hist) const;

	NeatPlotting::Booky create2DFitResultPlots(
			const PndLmdFitDataBundle &fit_data_bundle);

	/*	 std::map<LumiFit::PndLmdFitModelOptions,
	 std::map<std::set<LumiFit::LmdDimension>,
	 std::vector<PndLmdLumiHelper::lmd_graph*> > > sortLmdGraphs(
	 const std::vector<PndLmdLumiHelper::lmd_graph*> &graphs);

	 void createResolutionParameterizationDepedencyOverviewPlots(
	 std::map<std::set<LumiFit::LmdDimension>,
	 std::vector<PndLmdLumiHelper::lmd_graph*> > &data_selection_sorted_graphs,
	 TString dependency_name);

	 void createResolutionParameterizationComparisonPlots(
	 std::vector<PndLmdLumiHelper::lmd_graph*> &graphs,
	 std::vector<PndLmdLumiHelper::lmd_graph*> &reference);

	 void makeFitResultBooky(
	 std::map<PndLmdLumiFitOptions,
	 std::map<int, NeatPlotting::PlotBundle1D>,
	 LumiFit::Comparisons::fit_options_compare> &graph_bundle_map,
	 TString filename = "fitresults");


	 void makeResolutionSummaryPlots(const std::string &file_url);

	 void makeResolutionBooky(std::vector<PndLmdHistogramData> &res_vec,
	 TString filename);

	 void makeResolutionDifferencesBooky(std::vector<PndLmdHistogramData> &res_vec,
	 std::vector<PndLmdHistogramData> &res_vec_ref);

	 void makeComparisonCanvas(TString name,
	 std::map<TString, std::vector<NeatPlotting::GraphPoint> >& result_map);


	 void plotIPDependencyGraphs(
	 std::map<LumiFit::LmdSimIPParameters,
	 std::map<LumiFit::LmdSimIPParameters,
	 std::map<LumiFit::LmdDimensionType, std::vector<PndLmdVertexData> > > > &ip_data_clusters);
	 */
};
}

#endif /* PNDLMDPLOTTER_H_ */
