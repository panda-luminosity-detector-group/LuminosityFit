/*
 * PndLmdLumiFitBase.h
 *
 *  Created on: Jun 27, 2012
 *      Author: steve
 */

#ifndef PNDLMDLUMIHELPER_H_
#define PNDLMDLUMIHELPER_H_

#if !defined(__CINT__)

#include "SharedPtr.h"

#endif /* __CINT __ */

#include "LumiFitStructs.h"

#include <vector>
#include <map>

class ModelFitResult;
class PndLmdHistogramData;

class TH1D;
class TH2D;

/**
 * \brief This class contains a few functions helping the user with some basics.
 *
 * Main use of this class will be the actual interface to the data created by the lmd macros for filling #PndLmdDataBase objects.
 */
class PndLmdLumiHelper {
	struct findByDimensionType {
		findByDimensionType(const LumiFit::LmdDimensionType &dim_type_) :
				dim_type(dim_type_) {
		}
		bool operator()(const LumiFit::LmdDimension &lmd_dim) {
			return lmd_dim.dimension_options.dimension_type == dim_type;
		}
	private:
		LumiFit::LmdDimensionType dim_type;
	};
public:
	PndLmdLumiHelper();

	static double getMomentumTransferFromTheta(double plab, double theta);

	double calcHistIntegral(const TH1D* hist,
			std::vector<DataStructs::DimensionRange> range);
	double calcHistIntegral(const TH2D* hist,
			std::vector<DataStructs::DimensionRange> range);

#ifndef __CINT__
	std::map<double, ModelFitResult*> checkFitParameters(
			const std::map<PndLmdHistogramData*, ModelFitResult*> &fit_results) const;
#endif /* __CINT __ */
};

#endif /* PNDLMDLUMIHELPER_H_ */
