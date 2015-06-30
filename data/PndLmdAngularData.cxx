/*
 * PndLmdAngularData.h
 *
 *  Created on: Jun 27, 2012
 *      Author: steve
 */
#include "PndLmdAngularData.h"

ClassImp(PndLmdAngularData)

PndLmdAngularData::PndLmdAngularData() :
		reference_luminosity_per_event(1.0) {
}

PndLmdAngularData::PndLmdAngularData(const PndLmdAngularData &lmd_ang_data_) :
		PndLmdHistogramData(lmd_ang_data_) {
	reference_luminosity_per_event =
			lmd_ang_data_.getReferenceLuminosityPerEvent();
}

PndLmdAngularData::~PndLmdAngularData() {
}

double PndLmdAngularData::getReferenceLuminosity() const {
	return reference_luminosity_per_event * getNumEvents();
}

double PndLmdAngularData::getReferenceLuminosityPerEvent() const {
	return reference_luminosity_per_event;
}

void PndLmdAngularData::setReferenceLuminosityPerEvent(
		double reference_luminosity_per_event_) {
	reference_luminosity_per_event = reference_luminosity_per_event_;
}

bool PndLmdAngularData::operator<(const PndLmdAngularData &lmd_data) const {
	if (reference_luminosity_per_event
			< lmd_data.getReferenceLuminosityPerEvent())
		return true;
	else if (reference_luminosity_per_event
			> lmd_data.getReferenceLuminosityPerEvent())
		return false;
	return PndLmdAbstractData::operator<(lmd_data);
}
bool PndLmdAngularData::operator>(const PndLmdAngularData &lmd_data) const {
	return (lmd_data < *this);
}
bool PndLmdAngularData::operator==(const PndLmdAngularData &lmd_data) const {
	if (reference_luminosity_per_event
			!= lmd_data.getReferenceLuminosityPerEvent())
		return false;
	return PndLmdAbstractData::operator==(lmd_data);
}
bool PndLmdAngularData::operator!=(const PndLmdAngularData &lmd_data) const {
	return !(*this == lmd_data);
}
