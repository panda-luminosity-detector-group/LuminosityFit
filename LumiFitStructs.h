/*
 * LumiFitStructs.h
 *
 *  Created on: Jul 9, 2013
 *      Author: steve
 */

#ifndef LUMIFITSTRUCTS_H_
#define LUMIFITSTRUCTS_H_

#include "fit/data/DataStructs.h"
#include "core/ModelStructs.h"
#include "fit/EstimatorOptions.h"

#include <iostream>
#include <string>
#include <sstream>
#include <set>

#ifndef __CINT__
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#endif

#include "TObject.h"
#include "TString.h"

namespace LumiFit {

enum LmdEstimatorType {
	CHI2, LOG_LIKELIHOOD
};

#ifndef __CINT__
const boost::unordered_map<std::string, LmdEstimatorType> StringToLmdEstimatorType =
		boost::assign::map_list_of("CHI2", CHI2)("LOG_LIKELIHOOD", LOG_LIKELIHOOD);

const boost::unordered_map<LmdEstimatorType, std::string> LmdEstimatorTypeToString =
		boost::assign::map_list_of(CHI2, "CHI2")(LOG_LIKELIHOOD, "LOG_LIKELIHOOD");
#endif

enum LmdDataType {
	HISTOGRAM, EFFICIENCY
};

enum LmdDimensionType {
	X,
	Y,
	Z,
	T,
	THETA,
	PHI,
	THETA_X,
	THETA_Y,
	PARTICLE_ID,
	SECONDARY
};

#ifndef __CINT__
const boost::unordered_map<std::string, LmdDimensionType> StringToDimensionType =
		boost::assign::map_list_of("X", X)("Y", Y)("Z", Z)("T", T)("THETA", THETA)(
				"PHI", PHI)("THETA_X", THETA_X)("THETA_Y", THETA_Y)("PARTICLE_ID", PARTICLE_ID)(
				"SECONDARY", SECONDARY);
#endif

enum LmdTrackType {
	MC, MC_ACC, RECO, DIFF_RECO_MC
};

enum LmdTrackParamType {
	IP, LMD
};

#ifndef __CINT__
const boost::unordered_map<std::string, LmdTrackParamType> StringToTrackParamType =
		boost::assign::map_list_of("IP", IP)("LMD", LMD);
#endif

enum ModelType {
	GAUSSIAN, DOUBLE_GAUSSIAN, ASYMMETRIC_GAUSSIAN, UNIFORM
};

#ifndef __CINT__
const boost::unordered_map<std::string, ModelType> StringToModelType =
		boost::assign::map_list_of("GAUSSIAN", GAUSSIAN)("DOUBLE_GAUSSIAN",
				DOUBLE_GAUSSIAN)("ASYMMETRIC_GAUSSIAN", ASYMMETRIC_GAUSSIAN)("UNIFORM",
				UNIFORM);
#endif

#ifndef __CINT__
const boost::unordered_map<std::string, ModelStructs::InterpolationType> StringToInterpolationType =
		boost::assign::map_list_of("CONSTANT", ModelStructs::CONSTANT)("LINEAR",
				ModelStructs::LINEAR)("SPLINE", ModelStructs::SPLINE);
#endif

enum DPMElasticParts {
	COUL, INT, HAD, HAD_RHO_B_SIGTOT, ALL_RHO_B_SIGTOT, ALL
};

#ifndef __CINT__
const boost::unordered_map<std::string, DPMElasticParts> StringToDPMElasticParts =
		boost::assign::map_list_of("COUL", COUL)("INT", INT)("HAD", HAD)(
				"HAD_RHO_B_SIGTOT", HAD_RHO_B_SIGTOT)("ALL_RHO_B_SIGTOT",
				ALL_RHO_B_SIGTOT)("ALL", ALL);

const boost::unordered_map<DPMElasticParts, std::string> DPMElasticPartsToString =
		boost::assign::map_list_of(COUL, "COUL")(INT, "INT")(HAD, "HAD")(
				HAD_RHO_B_SIGTOT, "HAD_RHO_B_SIGTOT")(ALL_RHO_B_SIGTOT,
				"ALL_RHO_B_SIGTOT")(ALL, "ALL");

#endif

struct LmdDimensionOptions: public TObject {
	LmdDimensionType dimension_type;

	LmdTrackType track_type;

	LmdTrackParamType track_param_type;

	LmdDimensionOptions() {
		dimension_type = THETA;
		track_param_type = IP;
		track_type = RECO;
	}

	bool operator<(const LmdDimensionOptions &rhs) const {
		// check binary options first
		if (dimension_type < rhs.dimension_type)
			return true;
		else if (dimension_type > rhs.dimension_type)
			return false;
		if (track_type < rhs.track_type)
			return true;
		else if (track_type > rhs.track_type)
			return false;
		if (track_param_type < rhs.track_param_type)
			return true;
		else if (track_param_type > rhs.track_param_type)
			return false;

		return false;
	}

	bool operator>(const LmdDimensionOptions &rhs) const {
		return (rhs < *this);
	}

	/**
	 * This operator contains redundancy since it can be defined
	 * also from < (and >), but for reasons of speed this separate
	 * implementation exists.
	 */
	bool operator==(const LmdDimensionOptions &rhs) const {
		// check binary options first
		if (dimension_type != rhs.dimension_type)
			return false;
		else if (track_type != rhs.track_type)
			return false;
		else if (track_param_type != rhs.track_param_type)
			return false;

		return true;
	}

	bool operator!=(const LmdDimensionOptions &rhs) const {
		// check binary options first
		return !(*this == rhs);
	}

	friend std::ostream & operator <<(std::ostream & os,
			const LmdDimensionOptions & lmd_dim_opt) {
		os << lmd_dim_opt.dimension_type;
		os << "_" << lmd_dim_opt.track_type;
		os << "_" << lmd_dim_opt.track_param_type;

		return os;
	}
ClassDef(LmdDimensionOptions, 3)
	;
};

class LmdDimensionRange: public TObject {
private:
	// lower bound on this axis/dimension
	double range_low;
	// upper bound on this axis/dimension
	double range_high;

public:
	LmdDimensionRange() :
			range_low(0.0), range_high(0.0) {
	}

	double getDimensionLength() const {
		return (getRangeHigh() - getRangeLow());
	}

	double getDimensionMean() const {
		return (getRangeHigh() + getRangeLow()) / 2.0;
	}

	double getRangeLow() const {
		return range_low;
	}

	double getRangeHigh() const {
		return range_high;
	}

	void setRangeLow(double range_low_) {
		range_low = range_low_;
	}

	void setRangeHigh(double range_high_) {
		range_high = range_high_;
	}

	bool isDataWithinRange(double data_value) const {
		if (data_value < getRangeLow()) {
			return false;
		}
		if (data_value > getRangeHigh()) {
			return false;
		}
		return true;
	}

	bool operator<(const LmdDimensionRange &lmd_dim_range) const {
		if (range_low < lmd_dim_range.range_low)
			return true;
		else if (range_low > lmd_dim_range.range_low)
			return false;
		if (range_high < lmd_dim_range.range_high)
			return true;
		else if (range_high > lmd_dim_range.range_high)
			return false;

		return false;
	}

	bool operator>(const LmdDimensionRange &lmd_dim_range) const {
		return (lmd_dim_range < *this);
	}

	bool operator==(const LmdDimensionRange &lmd_dim_range) const {
		if (range_low != lmd_dim_range.range_low)
			return false;
		if (range_high != lmd_dim_range.range_high)
			return false;

		return true;
	}

	bool operator!=(const LmdDimensionRange &lmd_dim_range) const {
		return !(*this == lmd_dim_range);
	}

ClassDef(LmdDimensionRange, 1)
	;
};

struct BinDimension {
	DataStructs::DimensionRange x_range;
	DataStructs::DimensionRange y_range;

	BinDimension() {
	}

	BinDimension(DataStructs::DimensionRange x_range_,
			DataStructs::DimensionRange y_range_) :
			x_range(x_range_), y_range(y_range_) {
	}

	BinDimension(LmdDimensionRange x_range_, LmdDimensionRange y_range_) {
		x_range.range_low = x_range_.getRangeLow();
		x_range.range_high = x_range_.getRangeHigh();
		y_range.range_low = y_range_.getRangeLow();
		y_range.range_high = y_range_.getRangeHigh();
	}

	void printInfo() {
		std::cout << "x: " << x_range.range_low << " - " << x_range.range_high
				<< std::endl;
		std::cout << "y: " << y_range.range_low << " - " << y_range.range_high
				<< std::endl;
	}

	bool operator<(const BinDimension &rhs) const {
		if (x_range < rhs.x_range)
			return true;
		else if (x_range > rhs.x_range)
			return false;
		if (y_range < rhs.y_range)
			return true;
		else if (y_range > rhs.y_range)
			return false;

		return false;
	}
	bool operator>(const BinDimension &rhs) const {
		return (rhs < *this);
	}
};

inline double calculateBinOverlap(LumiFit::BinDimension &reco_bin,
		LumiFit::BinDimension &reso_bin) {
	LumiFit::BinDimension overlap;

	if (reco_bin.x_range.range_low > reso_bin.x_range.range_low)
		overlap.x_range.range_low = reco_bin.x_range.range_low;
	else
		overlap.x_range.range_low = reso_bin.x_range.range_low;

	if (reco_bin.y_range.range_low > reso_bin.y_range.range_low)
		overlap.y_range.range_low = reco_bin.y_range.range_low;
	else
		overlap.y_range.range_low = reso_bin.y_range.range_low;

	if (reco_bin.x_range.range_high < reso_bin.x_range.range_high)
		overlap.x_range.range_high = reco_bin.x_range.range_high;
	else
		overlap.x_range.range_high = reso_bin.x_range.range_high;

	if (reco_bin.y_range.range_high < reso_bin.y_range.range_high)
		overlap.y_range.range_high = reco_bin.y_range.range_high;
	else
		overlap.y_range.range_high = reso_bin.y_range.range_high;

	double reso_bin_area = (reso_bin.x_range.range_high
			- reso_bin.x_range.range_low)
			* (reso_bin.y_range.range_high - reso_bin.y_range.range_low);

	//reco_bin.printInfo();
	//reso_bin.printInfo();
	//overlap.printInfo();

	return (overlap.x_range.range_high - overlap.x_range.range_low)
			* (overlap.y_range.range_high - overlap.y_range.range_low) / reso_bin_area;
}

/**
 * Struct which defines a dimension of data.
 */
struct LmdDimension: public TObject {
	/**
	 * flag stating whether this dimension should be used or not.
	 * In its default initialization state it is set to false,
	 * either the user should set this flag manually to true or via
	 * #calculateBinSize() if this dimension should be used.
	 */
	bool is_active;
	/** the number of bins on this axis/dimension */
	unsigned int bins;

	/**
	 * Optional name for the dimension. This can be used for the
	 * id of the dimension, in case it is needed.
	 */
	std::string name;

	/**
	 * bin size = (range_high-range_low)/bins. This will be automatically
	 * calculated. Is required when using a binned fit to make fit result
	 * independent of the binning.
	 */
	double bin_size;

	LmdDimensionOptions dimension_options;

	LmdDimensionRange dimension_range;

	LmdDimension() {
		is_active = false;
		bins = 0;
		bin_size = 1.0;
	}

	void calculateBinSize() {
		is_active = true;
		bin_size = (dimension_range.getDimensionLength()) / bins;
	}

	LmdDimension clone() const {
		LmdDimension clone_object;
		clone_object.bins = bins;
		clone_object.dimension_range = dimension_range;
		clone_object.dimension_options = dimension_options;
		clone_object.calculateBinSize();
		return clone_object;
	}

	std::string createDimensionLabel() const {
		std::stringstream label;

		// var type
		if (dimension_options.dimension_type == X) {
			label << "x";
		} else if (dimension_options.dimension_type == Y) {
			label << "y";
		} else if (dimension_options.dimension_type == Z) {
			label << "z";
		} else if (dimension_options.dimension_type == T) {
			label << "|t|";
		} else if (dimension_options.dimension_type == THETA) {
			label << "#theta";
		} else if (dimension_options.dimension_type == PHI) {
			label << "#phi";
		} else if (dimension_options.dimension_type == THETA_X) {
			label << "#theta_{x}";
		} else if (dimension_options.dimension_type == THETA_Y) {
			label << "#theta_{y}";
		}

		// data type
		if (dimension_options.track_type == MC) {
			label << "^{MC}";
		} else if (dimension_options.track_type == MC_ACC) {
			label << "^{MC}";
		} else if (dimension_options.track_type == RECO) {
			label << "^{Rec}";
		} else if (dimension_options.track_type == DIFF_RECO_MC) {
			std::string temp(label.str());
			label << "^{Rec}-" << temp << "^{MC}";
		}

		return label.str();
	}

	std::string createUnitLabel() const {
		std::string unit;
		// var type
		if (dimension_options.dimension_type == X) {
			unit = "cm";
		} else if (dimension_options.dimension_type == Y) {
			unit = "cm";
		} else if (dimension_options.dimension_type == Z) {
			unit = "cm";
		} else if (dimension_options.dimension_type == T) {
			unit = "#frac{GeV^{2}}{c^{2}}";
		} else if (dimension_options.dimension_type == THETA
				|| dimension_options.dimension_type == THETA_X
				|| dimension_options.dimension_type == THETA_Y) {
			unit = "rad";
		} else if (dimension_options.dimension_type == PHI) {
			unit = "rad";
		}
		return unit;
	}

	/**
	 * Create label of this dimension which will be printed on the corresponding
	 * axis as labels. Generated from the #LmdDimensionType and #LmdTrackParamType.
	 */
	std::string createAxisLabel() const {
		std::stringstream label;

		if (dimension_options.dimension_type == THETA_X
				|| dimension_options.dimension_type == THETA_Y)
			label << createDimensionLabel() << " /" << createUnitLabel();
		else
			label << createDimensionLabel() << " /" << createUnitLabel();

		return label.str();
	}

	std::string createSelectionLabel() const {
		std::stringstream label;

		label << createDimensionLabel() << " = "
				<< dimension_range.getDimensionMean() << " " << createUnitLabel();

		return label.str();
	}

	bool operator<(const LmdDimension &lmd_dim) const {
		if (bins < lmd_dim.bins)
			return true;
		else if (bins > lmd_dim.bins)
			return false;
		if (dimension_range < lmd_dim.dimension_range)
			return true;
		else if (dimension_range > lmd_dim.dimension_range)
			return false;
		if (dimension_options < lmd_dim.dimension_options)
			return true;

		return false;
	}

	bool operator>(const LmdDimension &lmd_dim) const {
		return (lmd_dim < *this);
	}

	bool operator==(const LmdDimension &lmd_dim) const {
		if (bins != lmd_dim.bins)
			return false;
		if (dimension_range != lmd_dim.dimension_range)
			return false;
		if (dimension_options != lmd_dim.dimension_options)
			return false;

		return true;
	}

	bool operator!=(const LmdDimension &lmd_dim) const {
		return !(*this == lmd_dim);
	}

	friend std::ostream & operator <<(std::ostream & os,
			const LmdDimension & lmd_dim) {
		os << lmd_dim.dimension_options;
		os << "_" << lmd_dim.bins;
		os << "_" << lmd_dim.dimension_range.getRangeLow() << "-"
				<< lmd_dim.dimension_range.getRangeHigh();

		return os;
	}

ClassDef(LmdDimension, 5)
	;
};

/*struct LmdSimIPParameters: public TObject {
	double offset_x_mean;
	double offset_x_width;
	double offset_y_mean;
	double offset_y_width;
	double offset_z_mean;
	double offset_z_width;

	double tilt_x_mean;
	double tilt_x_width;
	double tilt_y_mean;
	double tilt_y_width;

	void print() {
		std::cout << "simulation beam properties are:" << std::endl;
		std::cout << "mean X pos: \t" << offset_x_mean << std::endl;
		std::cout << "width X pos: \t" << offset_x_width << std::endl;
		std::cout << "mean Y pos: \t" << offset_y_mean << std::endl;
		std::cout << "width Y pos: \t" << offset_y_width << std::endl;
		std::cout << "mean Z pos: \t" << offset_z_mean << std::endl;
		std::cout << "width Z pos: \t" << offset_z_width << std::endl;
		std::cout << "--------------------------------------------------"
				<< std::endl;
		std::cout << "mean X tilt: \t" << tilt_x_mean << std::endl;
		std::cout << "width X tilt: \t" << tilt_x_width << std::endl;
		std::cout << "mean Y tilt: \t" << tilt_y_mean << std::endl;
		std::cout << "width Y tilt: \t" << tilt_y_width << std::endl;
	}

	LmdSimIPParameters() {
		reset();
	}

	void reset() {
		offset_x_mean = 0.0;
		offset_x_width = 0.0;
		offset_y_mean = 0.0;
		offset_y_width = 0.0;
		offset_z_mean = 0.0;
		offset_z_width = 0.0;

		tilt_x_mean = 0.0;
		tilt_x_width = 0.0;
		tilt_y_mean = 0.0;
		tilt_y_width = 0.0;
	}

	std::string getLabel() const {
		std::stringstream ss;
		ss << "IP_pos_" << offset_x_mean << "_" << offset_x_width << "_"
				<< offset_y_mean << "_" << offset_y_width << "_" << offset_z_mean << "_"
				<< offset_z_width << "-beam_tilt_" << tilt_x_mean << "_" << tilt_x_width
				<< "_" << tilt_y_mean << "_" << tilt_y_width;
		return ss.str();
	}

	std::string getDependencyLabel() const {
		if (1.0 == offset_x_mean)
			return std::string("offset_x");
		if (1.0 == offset_x_width)
			return std::string("width_x");
		if (1.0 == offset_y_mean)
			return std::string("offset_y");
		if (1.0 == offset_y_width)
			return std::string("width_y");
		if (1.0 == offset_z_mean)
			return std::string("offset_z");
		if (1.0 == offset_z_width)
			return std::string("width_z");

		if (1.0 == tilt_x_mean)
			return std::string("tilt_x");
		if (1.0 == tilt_x_width)
			return std::string("div_x");
		if (1.0 == tilt_y_mean)
			return std::string("tilt_y");
		if (1.0 == tilt_y_width)
			return std::string("div_y");

		return std::string("");
	}

	double getDependencyValue(const LmdSimIPParameters &param_template) const {
		if (1.0 == param_template.offset_x_mean)
			return offset_x_mean;
		if (1.0 == param_template.offset_x_width)
			return offset_x_width;
		if (1.0 == param_template.offset_y_mean)
			return offset_y_mean;
		if (1.0 == param_template.offset_y_width)
			return offset_y_width;
		if (1.0 == param_template.offset_z_mean)
			return offset_z_mean;
		if (1.0 == param_template.offset_z_width)
			return offset_z_width;

		if (1.0 == param_template.tilt_x_mean)
			return tilt_x_mean;
		if (1.0 == param_template.tilt_x_width)
			return tilt_x_width;
		if (1.0 == param_template.tilt_y_mean)
			return tilt_y_mean;
		if (1.0 == param_template.tilt_y_width)
			return tilt_y_width;

		return 0.0;
	}

	std::string getDependencyName(
			const LmdSimIPParameters &param_template) const {
		std::string return_str("");
		if (1.0 == param_template.offset_x_mean)
			return_str = "#mu_{x}";
		else if (1.0 == param_template.offset_x_width)
			return_str = "#sigma_{x}";
		else if (1.0 == param_template.offset_y_mean)
			return_str = "#mu_{y}";
		else if (1.0 == param_template.offset_y_width)
			return_str = "#sigma_{y}";
		else if (1.0 == param_template.offset_z_mean)
			return_str = "#mu_{z}";
		else if (1.0 == param_template.offset_z_width)
			return_str = "#sigma_{z}";

		else if (1.0 == param_template.tilt_x_mean)
			return_str = "#mu_{x}";
		else if (1.0 == param_template.tilt_x_width)
			return_str = "#sigma_{x}";
		else if (1.0 == param_template.tilt_y_mean)
			return_str = "#mu_{x}";
		else if (1.0 == param_template.tilt_y_width)
			return_str = "#sigma_{x}";

		return return_str.append(" [cm]");
	}

	bool operator<(const LmdSimIPParameters &rhs) const {
		if (offset_x_mean < rhs.offset_x_mean)
			return true;
		else if (offset_x_mean > rhs.offset_x_mean)
			return false;
		if (offset_x_width < rhs.offset_x_width)
			return true;
		else if (offset_x_width > rhs.offset_x_width)
			return false;
		if (offset_y_mean < rhs.offset_y_mean)
			return true;
		else if (offset_y_mean > rhs.offset_y_mean)
			return false;
		if (offset_y_width < rhs.offset_y_width)
			return true;
		else if (offset_y_width > rhs.offset_y_width)
			return false;
		if (offset_z_mean < rhs.offset_z_mean)
			return true;
		else if (offset_z_mean > rhs.offset_z_mean)
			return false;
		if (offset_z_width < rhs.offset_z_width)
			return true;
		else if (offset_z_width > rhs.offset_z_width)
			return false;

		if (tilt_x_mean < rhs.tilt_x_mean)
			return true;
		else if (tilt_x_mean > rhs.tilt_x_mean)
			return false;
		if (tilt_x_width < rhs.tilt_x_width)
			return true;
		else if (tilt_x_width > rhs.tilt_x_width)
			return false;
		if (tilt_y_mean < rhs.tilt_y_mean)
			return true;
		else if (tilt_y_mean > rhs.tilt_y_mean)
			return false;
		if (tilt_y_width < rhs.tilt_y_width)
			return true;
		else if (tilt_y_width > rhs.tilt_y_width)
			return false;

		return false;
	}

	bool operator>(const LmdSimIPParameters &rhs) const {
		return (rhs < *this);
	}

ClassDef(LmdSimIPParameters ,1)
	;
};*/

}

#endif /* LUMIFITSTRUCTS_H_ */
