/*
 * ROOTPlotter.h
 *
 *  Created on: Jun 14, 2013
 *      Author: steve
 */

#ifndef ROOTPLOTTER_H_
#define ROOTPLOTTER_H_

#include "core/Model.h"

#include <memory>

class ModelVisualizationProperties1D;

class TGraphAsymmErrors;
class TH2D;

class ROOTPlotter {
private:
public:
	ROOTPlotter();
	virtual ~ROOTPlotter();

	TGraphAsymmErrors* createGraphFromModel1D(std::shared_ptr<Model> model,
			ModelVisualizationProperties1D &visualization_properties) const;

	TH2D* createHistogramFromModel2D(std::shared_ptr<Model> model,
			std::pair<ModelVisualizationProperties1D, ModelVisualizationProperties1D> &visualization_properties) const;
};

#endif /* ROOTPLOTTER_H_ */
