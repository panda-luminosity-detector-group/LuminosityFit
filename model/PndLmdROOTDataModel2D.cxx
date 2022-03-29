/*
 * PndLmdROOTDataModel1D.cpp
 *
 *  Created on: Jan 20, 2013
 *      Author: steve
 */

#include "PndLmdROOTDataModel2D.h"

#include <cmath>
#include <iostream>

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TGraph2DErrors.h"
#include "TH2D.h"

PndLmdROOTDataModel2D::PndLmdROOTDataModel2D(std::string name_)
    : Model2D(name_), using_acceptance_bounds(false), eff(0),
      acc_x_range_low(0.0), acc_x_range_high(0.0), acc_y_range_low(0.0),
      acc_y_range_high(0.0) {}

PndLmdROOTDataModel2D::~PndLmdROOTDataModel2D() {
  
}

void PndLmdROOTDataModel2D::updateDomainFromPars(double *par) {}

void PndLmdROOTDataModel2D::determineAcceptanceBounds() {
  double y = 0.0;

  acc_x_range_low = primary_data_dimension.getRangeLow();
  acc_x_range_high = primary_data_dimension.getRangeHigh();
  acc_y_range_low = secondary_data_dimension.getRangeLow();
  acc_y_range_high = secondary_data_dimension.getRangeHigh();

  TCanvas can;
  eff->Draw();
  can.Update();

  hist = (TH2D *)eff->GetPaintedHistogram();
  TH1D *projx = hist->ProjectionX();
  TH1D *projy = hist->ProjectionY();

  // scan lower for lower edge
  for (int i = 1; i < projx->GetXaxis()->GetNbins(); i++) {
    y = projx->GetBinContent(i);
    if (y > 1e-4) {
      acc_x_range_low = projx->GetBinLowEdge(i);
      break;
    }
  }
  // scan lower for upper edge
  for (int i = projx->GetXaxis()->GetNbins() - 1; i >= 0; i--) {
    y = projx->GetBinContent(i);
    if (y > 1e-4) {
      acc_x_range_high = projx->GetBinLowEdge(i) + projx->GetBinWidth(i);
      break;
    }
  }

  // scan lower for lower edge
  for (int i = 1; i < projy->GetXaxis()->GetNbins(); i++) {
    y = projy->GetBinContent(i);
    if (y > 1e-4) {
      acc_y_range_low = projy->GetBinLowEdge(i);
      break;
    }
  }
  // scan lower for upper edge
  for (int i = projy->GetXaxis()->GetNbins() - 1; i >= 0; i--) {
    y = projy->GetBinContent(i);
    if (y > 1e-4) {
      acc_y_range_high = projy->GetBinLowEdge(i) + projx->GetBinWidth(i);
      break;
    }
  }

  if (acc_x_range_high < acc_x_range_low) {
    acc_x_range_low = projx->GetBinLowEdge(1);
    acc_x_range_high = projx->GetBinLowEdge(projx->GetXaxis()->GetNbins() - 1) +
                       projx->GetBinWidth(projx->GetXaxis()->GetNbins() - 1);
  }
  if (acc_y_range_high < acc_y_range_low) {
    acc_y_range_low = projy->GetBinLowEdge(1);
    acc_y_range_high = projy->GetBinLowEdge(projx->GetXaxis()->GetNbins() - 1) +
                       projx->GetBinWidth(projy->GetXaxis()->GetNbins() - 1);
  }

  std::cout << "Calculated acceptance bounds to" << std::endl;
  std::cout << "x: " << acc_x_range_low << " and " << acc_x_range_high
            << std::endl;
  std::cout << "y: " << acc_y_range_low << " and " << acc_y_range_high
            << std::endl;
  updateDomain();
  using_acceptance_bounds = true;
}

std::pair<LumiFit::LmdDimensionRange, LumiFit::LmdDimensionRange>
PndLmdROOTDataModel2D::getDataDimension() const {
  return std::make_pair(primary_data_dimension, secondary_data_dimension);
}

void PndLmdROOTDataModel2D::setDataDimension(
    std::pair<LumiFit::LmdDimensionRange, LumiFit::LmdDimensionRange>
        data_dimension_) {
  primary_data_dimension = data_dimension_.first;
  secondary_data_dimension = data_dimension_.second;
  determineAcceptanceBounds();
}

TEfficiency *PndLmdROOTDataModel2D::getEfficieny() const { return eff; }

void PndLmdROOTDataModel2D::setEfficiency(TEfficiency *eff_) {
  eff = eff_;

  if (intpol_type == ModelStructs::CONSTANT) {
    model_func = &PndLmdROOTDataModel2D::evaluateConstant;
  }
}

TGraph2DErrors *PndLmdROOTDataModel2D::getGraph() const { return graph; }

void PndLmdROOTDataModel2D::setGraph(TGraph2DErrors *graph_) {
  graph = graph_;

  acc_x_range_low = graph->GetXmin();
  acc_x_range_high = graph->GetXmax();
  acc_y_range_low = graph->GetYmin();
  acc_y_range_high = graph->GetYmax();

  hist = graph->GetHistogram();

  if (intpol_type == ModelStructs::CONSTANT) {
    if (hist)
      model_func = &PndLmdROOTDataModel2D::evaluateGraphConstant;
    else
      model_func = &PndLmdROOTDataModel2D::evaluateConstant;
  }
}

ModelStructs::InterpolationType PndLmdROOTDataModel2D::getIntpolType() const {
  return intpol_type;
}

void PndLmdROOTDataModel2D::setIntpolType(
    ModelStructs::InterpolationType intpol_type_) {
  intpol_type = intpol_type_;
  if (intpol_type == ModelStructs::CONSTANT) {
    if (hist)
      model_func = &PndLmdROOTDataModel2D::evaluateGraphConstant;
    else
      model_func = &PndLmdROOTDataModel2D::evaluateConstant;
  } else if (intpol_type == ModelStructs::SPLINE) {
    model_func = &PndLmdROOTDataModel2D::evaluateSpline;
  } else {
    model_func = &PndLmdROOTDataModel2D::evaluateLinear;
  }
}

void PndLmdROOTDataModel2D::initModelParameters() {}

std::pair<std::pair<double, double>, std::pair<double, double>>
PndLmdROOTDataModel2D::getAcceptanceBounds() const {
  return std::make_pair(std::make_pair(acc_x_range_low, acc_x_range_high),
                        std::make_pair(acc_y_range_low, acc_y_range_high));
}

double PndLmdROOTDataModel2D::evaluateConstant(const double *x) const {
  Int_t closest = eff->FindFixBin(x[0], x[1]);

  return eff->GetEfficiency(closest);
}

double PndLmdROOTDataModel2D::evaluateGraphConstant(const double *x) const {
  return hist->Interpolate(x[0], x[1]);
}

double PndLmdROOTDataModel2D::evaluateLinear(const double *x) const {
  return graph->Interpolate(x[0], x[1]);
}

double PndLmdROOTDataModel2D::evaluateSpline(const double *x) const {
  // spline interpolation creating a new spline
  return graph->Interpolate(x[0], x[1]);
}

mydouble PndLmdROOTDataModel2D::eval(const double *x) const {
  if (acc_x_range_low > x[0] || acc_x_range_high < x[0] ||
      acc_y_range_low > x[1] || acc_y_range_high < x[1])
    return 0.0;
  return (this->*model_func)(x);
}

std::pair<double, double>
PndLmdROOTDataModel2D::getUncertaincy(const double *x) const {
  // check if we are outside of acceptance...
  if (eval(x) == 0.0)
    return std::make_pair(0.0, 0.0);
  // two nearest neighbors
  Int_t closest = eff->FindFixBin(x[0], x[1]);

  return std::make_pair(eff->GetEfficiencyErrorLow(closest),
                        eff->GetEfficiencyErrorUp(closest));
}

void PndLmdROOTDataModel2D::updateDomain() {
  setVar1Domain(acc_x_range_low, acc_x_range_high);
  setVar2Domain(acc_y_range_low, acc_y_range_high);
}
