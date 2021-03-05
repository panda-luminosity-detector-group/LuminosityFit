/*
 * PndLmdROOTDataModel1D.h
 *
 *  Created on: Jan 20, 2013
 *      Author: steve
 */

#ifndef PNDLMDROOTDATAMODEL1D_H_
#define PNDLMDROOTDATAMODEL1D_H_

#include "LumiFitStructs.h"
#include "core/Model1D.h"

class TEfficiency;
class TGraphAsymmErrors;
class TSpline3;

class PndLmdROOTDataModel1D : public Model1D {
private:
  LumiFit::LmdDimensionRange data_dimension;
  TGraphAsymmErrors *graph;
  TSpline3 *spline;
  mydouble acc_range_low, acc_range_high;
  bool using_acceptance_bounds;

  ModelStructs::InterpolationType intpol_type;

  void updateDomainFromPars(mydouble *par);

  void determineAcceptanceBounds();

  // function pointer used to switch between different algorithms for
  // interpolation
  typedef mydouble (PndLmdROOTDataModel1D::*function)(const mydouble *x) const;

  function model_func;

public:
  PndLmdROOTDataModel1D(std::string name_);
  virtual ~PndLmdROOTDataModel1D();

  LumiFit::LmdDimensionRange getDataDimension() const;
  void setDataDimension(LumiFit::LmdDimensionRange data_dimension_);

  TGraphAsymmErrors *getGraph() const;
  ModelStructs::InterpolationType getIntpolType() const;
  void setGraph(TGraphAsymmErrors *graph_);
  void setIntpolType(ModelStructs::InterpolationType intpol_type_);

  void initModelParameters();

  virtual std::pair<mydouble, mydouble> getUncertaincy(const mydouble *x) const;

  mydouble eval(const mydouble *x) const;

  std::pair<mydouble, mydouble> getAcceptanceBounds() const;
  void setAcceptanceBounds(mydouble low, mydouble high);

  mydouble evaluateConstant(const mydouble *x) const;
  mydouble evaluateLinear(const mydouble *x) const;
  mydouble evaluateSpline(const mydouble *x) const;

  void updateDomain();
};

#endif /* PNDLMDROOTDATAMODEL1D_H_ */
