/*
 * PndLmdDivergenceSmearingModel2D.h
 *
 *  Created on: Nov 21, 2014
 *      Author: steve
 */

#ifndef PNDLMDDIVERGENCESMEARINGMODEL2D_H_
#define PNDLMDDIVERGENCESMEARINGMODEL2D_H_

#include <core/Model2D.h>

#include "LumiFitStructs.h"

struct DifferentialCoordinateContribution {
	std::pair<int, int> coordinate_delta;
	double contribution_factor;

	DifferentialCoordinateContribution() :
			coordinate_delta(0, 0), contribution_factor(0.0) {
	}

	DifferentialCoordinateContribution(int x, int y, double weight) :
			coordinate_delta(x, y), contribution_factor(weight) {
	}

	bool operator==(const DifferentialCoordinateContribution &rhs) const {
		return coordinate_delta == rhs.coordinate_delta;
	}
};

class PndLmdDivergenceSmearingModel2D {
	shared_ptr<Model2D> divergence_model;

	LumiFit::LmdDimension data_dim_x;
	LumiFit::LmdDimension data_dim_y;

	std::vector<DifferentialCoordinateContribution> list_of_contributors;

	void generate2DDivergenceMap();

public:
	struct IntPairLess {
		bool operator()(const DifferentialCoordinateContribution &lhs,
				const DifferentialCoordinateContribution &rhs) {
			return lhs.coordinate_delta < rhs.coordinate_delta;
		}
	};

	PndLmdDivergenceSmearingModel2D(shared_ptr<Model2D> divergence_model_,
			const LumiFit::LmdDimension& data_dim_x_,
			const LumiFit::LmdDimension& data_dim_y_);
	virtual ~PndLmdDivergenceSmearingModel2D();

	std::pair<double, double> getBinsizes() const;

	const std::vector<DifferentialCoordinateContribution>& getListOfContributors(
			const double *x) const;

	void updateSmearingModel();
};

#endif /* PNDLMDDIVERGENCESMEARINGMODEL2D_H_ */
