/*
 * PndLmdROOTDataModel2D.h
 *
 *  Created on: Jan 20, 2013
 *      Author: steve
 */

#ifndef PNDLMDROOTDATAMODEL2D_H_
#define PNDLMDROOTDATAMODEL2D_H_

#include "core/Model2D.h"
#include "LumiFitStructs.h"

class TEfficiency;
class TGraph2DErrors;
class TH2D;

class PndLmdROOTDataModel2D: public Model2D {
private:
	LumiFit::LmdDimensionRange primary_data_dimension;
	LumiFit::LmdDimensionRange secondary_data_dimension;
	TEfficiency *eff;
	TGraph2DErrors *graph;
	TH2D* hist;
	double acc_x_range_low, acc_x_range_high, acc_y_range_low, acc_y_range_high;
	bool using_acceptance_bounds;

	ModelStructs::InterpolationType intpol_type;

	void updateDomainFromPars(double *par);

	void determineAcceptanceBounds();

	// function pointer used to switch between different algorithms for interpolation
	typedef double (PndLmdROOTDataModel2D::*function)(const double *x) const;

	function model_func;

public:
	PndLmdROOTDataModel2D(std::string name_);
	virtual ~PndLmdROOTDataModel2D();

	std::pair<LumiFit::LmdDimensionRange, LumiFit::LmdDimensionRange> getDataDimension() const;
	void setDataDimension(
			std::pair<LumiFit::LmdDimensionRange, LumiFit::LmdDimensionRange> data_dimension_);

	TGraph2DErrors *getGraph() const;
	void setGraph(TGraph2DErrors *graph_);

	TEfficiency *getEfficieny() const;
	void setEfficiency(TEfficiency *eff_);

	ModelStructs::InterpolationType getIntpolType() const;
	void setIntpolType(ModelStructs::InterpolationType intpol_type_);

	void initModelParameters();

	virtual std::pair<double, double> getUncertaincy(const double *x) const;

	double eval(const double *x) const;

	std::pair<std::pair<double, double>, std::pair<double, double> > getAcceptanceBounds() const;

	double evaluateConstant(const double *x) const;
	double evaluateGraphConstant(const double *x) const;
	double evaluateLinear(const double *x) const;
	double evaluateSpline(const double *x) const;

	void updateDomain();
};

#endif /* PNDLMDROOTDATAMODEL2D_H_ */
