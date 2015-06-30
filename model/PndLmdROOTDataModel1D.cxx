/*
 * PndLmdROOTDataModel1D.cpp
 *
 *  Created on: Jan 20, 2013
 *      Author: steve
 */

#include "PndLmdROOTDataModel1D.h"

#include <iostream>
#include <cmath>

#include "TGraphAsymmErrors.h"
#include "TSpline.h"

PndLmdROOTDataModel1D::PndLmdROOTDataModel1D(std::string name_) :
		Model1D(name_), spline(0), using_acceptance_bounds(false) {
}

PndLmdROOTDataModel1D::~PndLmdROOTDataModel1D() {
	// TODO Auto-generated destructor stub
}

void PndLmdROOTDataModel1D::updateDomainFromPars(double *par) {

}

void PndLmdROOTDataModel1D::determineAcceptanceBounds() {
	double y = 0.0;

	acc_range_low = data_dimension.getRangeLow();
	acc_range_high = data_dimension.getRangeHigh();

	//scan lower for lower edge
	for (int i = 1; i < graph->GetN(); i++) {
		y = graph->GetY()[i];
		if (y > 1e-4) {
			acc_range_low = graph->GetX()[i - 1];
			break;
		}
	}
	//scan lower for upper edge
	for (int i = graph->GetN() - 2; i >= 0; i--) {
		y = graph->GetY()[i];
		if (y > 1e-4) {
			acc_range_high = graph->GetX()[i + 1];
			break;
		}
	}

	if (acc_range_high < acc_range_low)
		acc_range_high = graph->GetX()[graph->GetN() - 1];
	std::cout << "Calculated acceptance bounds to " << acc_range_low << " and "
			<< acc_range_high << std::endl;
	setDomain(acc_range_low, acc_range_high);
	using_acceptance_bounds = true;
}

LumiFit::LmdDimensionRange PndLmdROOTDataModel1D::getDataDimension() const {
	return data_dimension;
}

void PndLmdROOTDataModel1D::setDataDimension(
		LumiFit::LmdDimensionRange data_dimension_) {
	data_dimension = data_dimension_;
	determineAcceptanceBounds();
}

TGraphAsymmErrors *PndLmdROOTDataModel1D::getGraph() const {
	return graph;
}

ModelStructs::InterpolationType PndLmdROOTDataModel1D::getIntpolType() const {
	return intpol_type;
}

void PndLmdROOTDataModel1D::setGraph(TGraphAsymmErrors *graph_) {
	graph = graph_;

	int low = 0;
	int high = graph->GetN() - 1;

	acc_range_low = graph->GetX()[low];
	acc_range_high = graph->GetX()[high];

	if (0 == spline)
		spline = new TSpline3("acc_spline", graph->GetX(), graph->GetY(),
				graph->GetN());
}

void PndLmdROOTDataModel1D::setIntpolType(
		ModelStructs::InterpolationType intpol_type_) {
	intpol_type = intpol_type_;
	if (intpol_type == ModelStructs::CONSTANT) {
		model_func = &PndLmdROOTDataModel1D::evaluateConstant;
	} else if (intpol_type == ModelStructs::SPLINE) {
		model_func = &PndLmdROOTDataModel1D::evaluateSpline;
	} else {
		model_func = &PndLmdROOTDataModel1D::evaluateLinear;
	}
}

void PndLmdROOTDataModel1D::initModelParameters() {

}

std::pair<double, double> PndLmdROOTDataModel1D::getAcceptanceBounds() const {
	return std::make_pair(acc_range_low, acc_range_high);
}

void PndLmdROOTDataModel1D::setAcceptanceBounds(double low, double high) {
	acc_range_low = low;
	acc_range_high = high;
}

double PndLmdROOTDataModel1D::evaluateConstant(const double *x) const {
	Int_t closest = -1;
	double diff = -1.0;

	for (Int_t i = 0; i < graph->GetN(); ++i) {
		if (diff < 0.0 || fabs(graph->GetX()[i] - x[0]) < diff) {
			diff = fabs(graph->GetX()[i] - x[0]);
			closest = i;
		}
	}
	return graph->GetY()[closest];
}

double PndLmdROOTDataModel1D::evaluateLinear(const double *x) const {
	return graph->Eval(x[0]);
}

double PndLmdROOTDataModel1D::evaluateSpline(const double *x) const {
	// spline interpolation creating a new spline
	return spline->Eval(x[0]);
}

double PndLmdROOTDataModel1D::eval(const double *x) const {
	if (acc_range_low > x[0] || acc_range_high < x[0])
		return 0.0;
	return (this->*model_func)(x);
}

std::pair<double, double> PndLmdROOTDataModel1D::getUncertaincy(
		const double *x) const {
	// check if we are outside of acceptance...
	if (eval(x) == 0.0)
		return std::make_pair(0.0, 0.0);
	// two nearest neighbors
	int closest_bin = -1;
	double diff = -1.0;

	for (Int_t i = 0; i < graph->GetN(); ++i) {
		if (diff < 0.0 || fabs(graph->GetX()[i] - x[0]) < diff) {
			diff = fabs(graph->GetX()[i] - x[0]);
			closest_bin = i;
		}
	}

	return std::make_pair(graph->GetErrorYlow(closest_bin),
			graph->GetErrorYhigh(closest_bin));
}

void PndLmdROOTDataModel1D::updateDomain() {
	setDomain(acc_range_low, acc_range_high);
}
