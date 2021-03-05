/*
 * ROOTPlotter.cxx
 *
 *  Created on: Jun 14, 2013
 *      Author: steve
 */

#include "ROOTPlotter.h"
#include "visualization/ModelVisualizationProperties1D.h"

#include "TGraphAsymmErrors.h"
#include "TH2D.h"

#include <cmath>
#include <iostream>
#include <limits>

ROOTPlotter::ROOTPlotter() {
  // TODO Auto-generated constructor stub
}

ROOTPlotter::~ROOTPlotter() {
  // TODO Auto-generated destructor stub
}

TGraphAsymmErrors *ROOTPlotter::createGraphFromModel1D(
    std::shared_ptr<Model> model,
    ModelVisualizationProperties1D &visualization_properties) const {

  TGraphAsymmErrors *graph =
      new TGraphAsymmErrors(visualization_properties.getEvaluations());

  if (model->init()) {
    std::cout << "Error: not all parameters have been set!" << std::endl;
  }

  mydouble stepsize =
      visualization_properties.getPlotRange().getDimensionLength() /
      visualization_properties.getEvaluations();
  mydouble x;

  for (unsigned int i = 0; i < visualization_properties.getEvaluations(); i++) {
    x = visualization_properties.getPlotRange().range_low + stepsize * i;
    graph->SetPoint(i, x,
                    model->evaluate(&x) *
                        visualization_properties.getBinningFactor());
    graph->SetPointError(i, 0, 0,
                         model->getUncertaincy(&x).first *
                             visualization_properties.getBinningFactor(),
                         model->getUncertaincy(&x).second *
                             visualization_properties.getBinningFactor());
  }
  return graph;
}

TH2D *ROOTPlotter::createHistogramFromModel2D(
    std::shared_ptr<Model> model,
    std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D>
        &visualization_properties) const {

  TH2D *hist = new TH2D(
      "model_hist", "", visualization_properties.first.getEvaluations(),
      visualization_properties.first.getPlotRange().range_low,
      visualization_properties.first.getPlotRange().range_high,
      visualization_properties.second.getEvaluations(),
      visualization_properties.second.getPlotRange().range_low,
      visualization_properties.second.getPlotRange().range_high);

  if (model->init()) {
    std::cout << "Error: not all parameters have been set!" << std::endl;
  }

  mydouble stepsize_x =
      visualization_properties.first.getPlotRange().getDimensionLength() /
      visualization_properties.first.getEvaluations();
  mydouble stepsize_y =
      visualization_properties.second.getPlotRange().getDimensionLength() /
      visualization_properties.second.getEvaluations();
  mydouble x[2];

  for (unsigned int ix = 0;
       ix < visualization_properties.first.getEvaluations(); ix++) {
    x[0] = visualization_properties.first.getPlotRange().range_low +
           stepsize_x * (1.0 * ix + 0.5);
    DataStructs::DimensionRange drx(x[0] - 0.5 * stepsize_x,
                                    x[0] + 0.5 * stepsize_x);
    for (unsigned int iy = 0;
         iy < visualization_properties.second.getEvaluations(); iy++) {
      x[1] = visualization_properties.second.getPlotRange().range_low +
             stepsize_y * (1.0 * iy + 0.5);

      DataStructs::DimensionRange dry(x[1] - 0.5 * stepsize_y,
                                      x[1] + 0.5 * stepsize_y);
      /*std::cout << "[" << x[0] << ", " << x[1] << "] -> "
       << model->evaluate(x)
       * visualization_properties.first.getBinningFactor()
       * visualization_properties.second.getBinningFactor() << std::endl;*/

      /*	std::vector<DataStructs::DimensionRange> int_ranges;
       int_ranges.push_back(drx);
       int_ranges.push_back(dry);

       double integral = model->Integral(int_ranges, 1e-3);

       hist->Fill(x[0], x[1], integral);*/

      mydouble value = model->evaluate(x);
      if (std::fabs(value) > std::numeric_limits<mydouble>::min())
        hist->Fill(x[0], x[1],
                   model->evaluate(x) *
                       visualization_properties.first.getBinningFactor() *
                       visualization_properties.second.getBinningFactor());
    }
  }
  return hist;
}
