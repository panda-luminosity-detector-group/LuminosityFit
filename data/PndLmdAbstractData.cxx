/*
 * PndLmdAbstractData.cxx
 *
 *  Created on: Apr 17, 2013
 *      Author: steve
 */

#include "PndLmdAbstractData.h"

#include <iostream>
#include <sstream>

#include "boost/lexical_cast.hpp"
#include "boost/property_tree/ptree.hpp"

using std::cout;
using std::endl;

ClassImp(PndLmdAbstractData)

    PndLmdAbstractData::PndLmdAbstractData()
    : p_lab(0.0) {}

PndLmdAbstractData::PndLmdAbstractData(const PndLmdAbstractData &lmd_abs_data_)
    : num_events(lmd_abs_data_.getNumEvents()),
      p_lab(lmd_abs_data_.getLabMomentum()), name(lmd_abs_data_.getName()),
      primary_dimension(lmd_abs_data_.getPrimaryDimension()),
      secondary_dimension(lmd_abs_data_.getSecondaryDimension()),
      selection_dimensions(lmd_abs_data_.getSelectorSet()) {}

PndLmdAbstractData::~PndLmdAbstractData() {
  // TODO Auto-generated destructor stub
}

int PndLmdAbstractData::getNumEvents() const { return num_events; }

double PndLmdAbstractData::getLabMomentum() const { return p_lab; }

const std::string &PndLmdAbstractData::getName() const { return name; }

const LumiFit::LmdDimension &PndLmdAbstractData::getPrimaryDimension() const {
  return primary_dimension;
}

const LumiFit::LmdDimension &PndLmdAbstractData::getSecondaryDimension() const {
  return secondary_dimension;
}

double PndLmdAbstractData::getBinningFactor(int dimension) const {
  double bin_factor = getPrimaryDimension().bin_size;
  if (getSecondaryDimension().is_active && dimension > 1)
    bin_factor *= getSecondaryDimension().bin_size;
  return bin_factor;
}

const std::set<LumiFit::LmdDimension> &
PndLmdAbstractData::getSelectorSet() const {
  return selection_dimensions;
}

void PndLmdAbstractData::addSelectionDimension(
    const LumiFit::LmdDimension &lmd_dim) {
  selection_dimensions.insert(lmd_dim);
}

void PndLmdAbstractData::setNumEvents(int num_events_) {
  num_events = num_events_;
}

void PndLmdAbstractData::setLabMomentum(double p_lab_) { p_lab = p_lab_; }

void PndLmdAbstractData::setName(const std::string &name_) { name = name_; }

void PndLmdAbstractData::setPrimaryDimension(
    const LumiFit::LmdDimension &primary_dimension_) {
  primary_dimension = primary_dimension_;
  init1DData();
}

void PndLmdAbstractData::setSecondaryDimension(
    const LumiFit::LmdDimension &secondary_dimension_) {
  secondary_dimension = secondary_dimension_;
  init2DData();
}

int PndLmdAbstractData::addFileToList(const std::string &filepath) {
  return filepath_list.insert(filepath).second;
}

void PndLmdAbstractData::saveToRootFile() {
  cout << "Saving " << getName() << " to current root file..." << endl;
  this->Write(getName().c_str());
}

bool PndLmdAbstractData::
operator<(const PndLmdAbstractData &rhs_lmd_data) const {
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

  return false;
}

bool PndLmdAbstractData::
operator>(const PndLmdAbstractData &rhs_lmd_data) const {
  return (rhs_lmd_data < *this);
}

bool PndLmdAbstractData::
operator==(const PndLmdAbstractData &rhs_lmd_data) const {

  // if (getNumEvents() != rhs_lmd_data.getNumEvents())
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

  return true;
}

bool PndLmdAbstractData::
operator!=(const PndLmdAbstractData &rhs_lmd_data) const {
  return !(*this == rhs_lmd_data);
}
