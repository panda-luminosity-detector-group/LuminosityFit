/*
 * PndLmdAngularData.h
 *
 *  Created on: Jun 27, 2012
 *      Author: steve
 */

#ifndef PNDLMDANGULARDATA_H_
#define PNDLMDANGULARDATA_H_

#include "PndLmdHistogramData.h"
#include "../LumiFitStructs.h"

/**
 * \brief Class used to describe 1D, 2D and unbinned LMD data, using ROOT
 * histograms and trees.
 * Important is that the user has to set the dimension variables first,
 * before any storage will automatically be allocated.
 */
class PndLmdAngularData: public PndLmdHistogramData {
private:
	/**
	 * In case this is a simulation a reference value for the luminosity
	 * can/should be used. This value corresponds to the luminosity per event,
	 * so multiplying by the number of events results in the actual integrated
	 * luminosity.
	 */
	double reference_luminosity_per_event;

public:
	PndLmdAngularData();
	PndLmdAngularData(const PndLmdAngularData &lmd_ang_data_);

	virtual ~PndLmdAngularData();

	double getReferenceLuminosity() const;
	double getReferenceLuminosityPerEvent() const;
	void setReferenceLuminosityPerEvent(double reference_luminosity_per_event_);

	bool operator<(const PndLmdAngularData &lmd_data) const;
	bool operator>(const PndLmdAngularData &lmd_data) const;
	bool operator==(const PndLmdAngularData &lmd_data) const;
	bool operator!=(const PndLmdAngularData &lmd_data) const;

ClassDef(PndLmdAngularData,2)
};

#endif /* PNDLMDANGULARDATA_H_ */
