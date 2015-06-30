/*
 * PndLmdAcceptance.cxx
 *
 *  Created on: Jul 4, 2012
 *      Author: steve
 */

#include "PndLmdAcceptance.h"

ClassImp(PndLmdAcceptance)

PndLmdAcceptance::PndLmdAcceptance() {
	name = "lmd_acceptance";
	acceptance_1d = 0;
	acceptance_2d = 0;
}

PndLmdAcceptance::PndLmdAcceptance(const PndLmdAcceptance &lmd_acc_data_) :
		PndLmdAbstractData(lmd_acc_data_) {
	if (lmd_acc_data_.getAcceptance1D())
		acceptance_1d = new TEfficiency(*lmd_acc_data_.getAcceptance1D());
	if (lmd_acc_data_.getAcceptance2D())
		acceptance_2d = new TEfficiency(*lmd_acc_data_.getAcceptance2D());
}

PndLmdAcceptance::~PndLmdAcceptance() {
}

void PndLmdAcceptance::init1DData() {
	// 1d acceptance
	acceptance_1d = new TEfficiency("acc1d", "", primary_dimension.bins,
			primary_dimension.dimension_range.getRangeLow(),
			primary_dimension.dimension_range.getRangeHigh());
}

void PndLmdAcceptance::init2DData() {
	// 2d acceptance
	acceptance_2d = new TEfficiency("acc2d", "", primary_dimension.bins,
			primary_dimension.dimension_range.getRangeLow(),
			primary_dimension.dimension_range.getRangeHigh(),
			secondary_dimension.bins,
			secondary_dimension.dimension_range.getRangeLow(),
			secondary_dimension.dimension_range.getRangeHigh());
}

void PndLmdAcceptance::cloneData(const PndLmdAbstractData &lmd_abs_data) {
	const PndLmdAcceptance * lmd_acc =
			dynamic_cast<const PndLmdAcceptance*>(&lmd_abs_data);
	if (lmd_acc) {
		acceptance_1d = new TEfficiency(*lmd_acc->getAcceptance1D());
		if (getSecondaryDimension().is_active) {
			acceptance_2d = new TEfficiency(*lmd_acc->getAcceptance2D());
		}
	}
}

TEfficiency* PndLmdAcceptance::getAcceptance1D() const {
	return acceptance_1d;
}
TEfficiency* PndLmdAcceptance::getAcceptance2D() const {
	return acceptance_2d;
}

void PndLmdAcceptance::add(const PndLmdAbstractData &lmd_abs_data_addition) {
	const PndLmdAcceptance * lmd_acc_addition =
			dynamic_cast<const PndLmdAcceptance*>(&lmd_abs_data_addition);
	if (lmd_acc_addition) {
		if (getPrimaryDimension().dimension_range
				== lmd_acc_addition->getPrimaryDimension().dimension_range) {
			setNumEvents(getNumEvents() + lmd_acc_addition->getNumEvents());
			acceptance_1d->Add(*lmd_acc_addition->getAcceptance1D());
			if (getSecondaryDimension().is_active) {
				if (getSecondaryDimension().dimension_range
						== lmd_acc_addition->getSecondaryDimension().dimension_range) {
					acceptance_2d->Add(*lmd_acc_addition->getAcceptance2D());
				}
			}
		}
	}
}

// acceptance filling methods
void PndLmdAcceptance::addData(bool is_accepted, double primary_value,
		double secondary_value) {
	acceptance_1d->Fill(is_accepted, primary_value);
	if (secondary_dimension.is_active)
		acceptance_2d->Fill(is_accepted, primary_value, secondary_value);
}
