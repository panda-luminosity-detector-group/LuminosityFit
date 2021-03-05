/*
 * ModelVisualizationProperties1D.cxx
 *
 *  Created on: Jun 15, 2013
 *      Author: steve
 */

#include "ModelVisualizationProperties1D.h"

ModelVisualizationProperties1D::ModelVisualizationProperties1D()
    : plot_range(), binning_factor(1.0), evaluations(500) {}

ModelVisualizationProperties1D::ModelVisualizationProperties1D(
    std::shared_ptr<Data> data)
    : plot_range(), binning_factor(data->getBinningFactor()), evaluations(500) {
}

double ModelVisualizationProperties1D::getBinningFactor() const {
  return binning_factor;
}

unsigned int ModelVisualizationProperties1D::getEvaluations() const {
  return evaluations;
}

DataStructs::DimensionRange
ModelVisualizationProperties1D::getPlotRange() const {
  return plot_range;
}

void ModelVisualizationProperties1D::setBinningFactor(double binning_factor_) {
  binning_factor = binning_factor_;
}

void ModelVisualizationProperties1D::setEvaluations(unsigned int evaluations_) {
  this->evaluations = evaluations_;
}

void ModelVisualizationProperties1D::setPlotRange(
    DataStructs::DimensionRange plot_range_) {
  plot_range = plot_range_;
}

ModelVisualizationProperties1D::~ModelVisualizationProperties1D() {
  // TODO Auto-generated destructor stub
}
