/*
 * PndLmdAbstractData.h
 *
 *  Created on: Apr 17, 2013
 *      Author: steve
 */

#ifndef PNDLMDABSTRACTDATA_H_
#define PNDLMDABSTRACTDATA_H_

#include "../LumiFitStructs.h"

#include <set>
#include <string>

#ifndef __CINT__
#include <boost/property_tree/ptree_fwd.hpp>
#endif /* __CINT __ */

#include "TObject.h"

class PndLmdAbstractData: public TObject {
private:
	/**
	 * The number of events requested to be filled to the histogram. Events may
	 * be outside of specified range (under/overflow). This is mainly used only
	 * for the histogram filling part (to know when to stop).
	 */
	int num_events;

	/** The lab momentum of the antiproton beam */
	double p_lab;

	/**
	 * List of filepaths that contain the data that was read in. Only used for
	 * bookkeeping/logging so later on it can be identified which data was used
	 * for this data object.
	 */
	std::set<std::string> filepath_list;

protected:
	/** Name of the object used for saving the object in a root file */
	std::string name;

	// we only allow 1d or 2d data objects here
	/** defines the primary dimension */
	LumiFit::LmdDimension primary_dimension;
	/** defines the secondary dimension (optional) */
	LumiFit::LmdDimension secondary_dimension;

	virtual void init1DData() =0;
	virtual void init2DData() =0;

	/**
	 * Unique list of dimensions that define the selections that will be
	 * performed on the input data
	 */
	std::set<LumiFit::LmdDimension> selection_dimensions;

	// usually we would just have the ptree here
	// unfortunately root cint is not capable to parse the ptree header
	// so this simpler option map was "developed"...
	std::map<std::string, std::string> simulation_parameters;

public:
	PndLmdAbstractData();
	PndLmdAbstractData(const PndLmdAbstractData &lmd_abs_data_);
	virtual ~PndLmdAbstractData();

	// getter methods
	int getNumEvents() const;
	double getLabMomentum() const;
	const std::string& getName() const;

	const LumiFit::LmdDimension& getPrimaryDimension() const;
	const LumiFit::LmdDimension& getSecondaryDimension() const;

	double getBinningFactor(int dimension) const;

	const std::set<LumiFit::LmdDimension>& getSelectorSet() const;

#ifndef __CINT__
	boost::property_tree::ptree getSimulationParametersPropertyTree() const;
#endif /* __CINT __ */

	// setter methods
	void setNumEvents(int num_events_);
	void setLabMomentum(double p_lab_);
	void setName(const std::string &name_);

	void setPrimaryDimension(const LumiFit::LmdDimension &primary_dimension_);
	void setSecondaryDimension(const LumiFit::LmdDimension &secondary_dimension_);

	void addSelectionDimension(const LumiFit::LmdDimension &lmd_dim);

#ifndef __CINT__
	void setSimulationParameters(
			const boost::property_tree::ptree &simulation_parameters_);
#endif /* __CINT __ */

	virtual void saveToRootFile();
	int addFileToList(const std::string &filepath);

	virtual void add(const PndLmdAbstractData &lmd_abs_data_addition) =0;

	virtual bool operator<(const PndLmdAbstractData &rhs_lmd_data) const;
	virtual bool operator>(const PndLmdAbstractData &rhs_lmd_data) const;
	virtual bool operator==(const PndLmdAbstractData &rhs_lmd_data) const;
	virtual bool operator!=(const PndLmdAbstractData &rhs_lmd_data) const;

ClassDef(PndLmdAbstractData,4)
};

#endif /* PNDLMDABSTRACTDATA_H_ */
