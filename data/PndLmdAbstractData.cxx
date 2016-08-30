/*
 * PndLmdAbstractData.cxx
 *
 *  Created on: Apr 17, 2013
 *      Author: steve
 */

#include "PndLmdAbstractData.h"

#include <iostream>
#include <sstream>

#include "boost/property_tree/ptree.hpp"
#include "boost/lexical_cast.hpp"

using std::cout;
using std::endl;

ClassImp(PndLmdAbstractData)

PndLmdAbstractData::PndLmdAbstractData() :
    p_lab(0.0) {
}

PndLmdAbstractData::PndLmdAbstractData(const PndLmdAbstractData &lmd_abs_data_) :
    num_events(lmd_abs_data_.getNumEvents()), p_lab(
        lmd_abs_data_.getLabMomentum()), name(lmd_abs_data_.getName()), primary_dimension(
        lmd_abs_data_.getPrimaryDimension()), secondary_dimension(
        lmd_abs_data_.getSecondaryDimension()), selection_dimensions(
        lmd_abs_data_.getSelectorSet()), simulation_parameters(
        lmd_abs_data_.simulation_parameters) {
}

PndLmdAbstractData::~PndLmdAbstractData() {
  // TODO Auto-generated destructor stub
}

int PndLmdAbstractData::getNumEvents() const {
  return num_events;
}

double PndLmdAbstractData::getLabMomentum() const {
  return p_lab;
}

const std::string& PndLmdAbstractData::getName() const {
  return name;
}

const LumiFit::LmdDimension& PndLmdAbstractData::getPrimaryDimension() const {
  return primary_dimension;
}

const LumiFit::LmdDimension& PndLmdAbstractData::getSecondaryDimension() const {
  return secondary_dimension;
}

double PndLmdAbstractData::getBinningFactor(int dimension) const {
  double bin_factor = getPrimaryDimension().bin_size;
  if (getSecondaryDimension().is_active && dimension > 1)
    bin_factor *= getSecondaryDimension().bin_size;
  return bin_factor;
}

const std::set<LumiFit::LmdDimension>& PndLmdAbstractData::getSelectorSet() const {
  return selection_dimensions;
}

boost::property_tree::ptree PndLmdAbstractData::getSimulationParametersPropertyTree() const {
  boost::property_tree::ptree model_opt_ptree;

  using boost::lexical_cast;
  using boost::bad_lexical_cast;

  std::map<std::string, std::string>::const_iterator simulation_parameter;
  for (simulation_parameter = simulation_parameters.begin();
      simulation_parameter != simulation_parameters.end();
      ++simulation_parameter) {
    bool success(false);
    // check if its a bool
    try {
      bool value = lexical_cast<bool>(simulation_parameter->second);
      model_opt_ptree.put(simulation_parameter->first, value);
      success = true;
    } catch (const bad_lexical_cast &) {
    }
    // check if its an int
    try {
      if (!success) {
        int value = lexical_cast<int>(simulation_parameter->second);
        model_opt_ptree.put(simulation_parameter->first, value);
        success = true;
      }
    } catch (const bad_lexical_cast &) {
    }
    // check if its a double
    try {
      if (!success) {
        double value = lexical_cast<double>(simulation_parameter->second);
        model_opt_ptree.put(simulation_parameter->first, value);
        success = true;
      }
    } catch (const bad_lexical_cast &) {
    }
    // otherwise just take it as a string
    model_opt_ptree.put(simulation_parameter->first,
        simulation_parameter->second);
  }

  return model_opt_ptree;
}

void PndLmdAbstractData::addSelectionDimension(
    const LumiFit::LmdDimension& lmd_dim) {
  selection_dimensions.insert(lmd_dim);
}

void PndLmdAbstractData::setNumEvents(int num_events_) {
  num_events = num_events_;
}

void PndLmdAbstractData::setLabMomentum(double p_lab_) {
  p_lab = p_lab_;
}

void PndLmdAbstractData::setName(const std::string& name_) {
  name = name_;
}

void PndLmdAbstractData::setPrimaryDimension(
    const LumiFit::LmdDimension& primary_dimension_) {
  primary_dimension = primary_dimension_;
  init1DData();
}

void PndLmdAbstractData::setSecondaryDimension(
    const LumiFit::LmdDimension& secondary_dimension_) {
  secondary_dimension = secondary_dimension_;
  init2DData();
}

void PndLmdAbstractData::setSimulationParameters(
    const boost::property_tree::ptree &simulation_parameters_) {
  boost::property_tree::ptree::const_iterator iter;
  // convert the ptree to simple format...
  for (iter = simulation_parameters_.begin();
      iter != simulation_parameters_.end(); iter++) {
    simulation_parameters[iter->first] = iter->second.data();
  }
}

int PndLmdAbstractData::addFileToList(const std::string& filepath) {
  return filepath_list.insert(filepath).second;
}

void PndLmdAbstractData::saveToRootFile() {
  cout << "Saving " << getName() << " to current root file..." << endl;
  this->Write(getName().c_str());
}

bool PndLmdAbstractData::operator<(
    const PndLmdAbstractData &rhs_lmd_data) const {
  // leave out num_events cuz that is a bit special
  if (getLabMomentum() < rhs_lmd_data.getLabMomentum())
    return true;
  else if (getLabMomentum() > rhs_lmd_data.getLabMomentum())
    return false;
  if (primary_dimension < rhs_lmd_data.primary_dimension)
    return true;
  else if (primary_dimension > rhs_lmd_data.primary_dimension)
    return false;
  if (secondary_dimension.is_active) {
    if (secondary_dimension < rhs_lmd_data.secondary_dimension)
      return true;
  }

  if (selection_dimensions < rhs_lmd_data.selection_dimensions)
    return true;
  else if (selection_dimensions > rhs_lmd_data.selection_dimensions) {
    return false;
  }

  if (simulation_parameters < rhs_lmd_data.simulation_parameters)
    return true;
  else if (simulation_parameters > rhs_lmd_data.simulation_parameters)
    return false;

  return false;
}

bool PndLmdAbstractData::operator>(
    const PndLmdAbstractData &rhs_lmd_data) const {
  return (rhs_lmd_data < *this);
}

bool PndLmdAbstractData::operator==(
    const PndLmdAbstractData &rhs_lmd_data) const {

  //if (getNumEvents() != rhs_lmd_data.getNumEvents())
  //	return false;
  if (getLabMomentum() != rhs_lmd_data.getLabMomentum())
    return false;
  if (primary_dimension != rhs_lmd_data.primary_dimension)
    return false;
  if (secondary_dimension.is_active) {
    if (secondary_dimension != rhs_lmd_data.secondary_dimension)
      return false;
  }

  if (selection_dimensions != rhs_lmd_data.selection_dimensions)
    return false;

  if (simulation_parameters != rhs_lmd_data.simulation_parameters)
    return false;

  return true;
}

bool PndLmdAbstractData::operator!=(
    const PndLmdAbstractData &rhs_lmd_data) const {
  return !(*this == rhs_lmd_data);
}
