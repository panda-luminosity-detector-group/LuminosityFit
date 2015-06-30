/*
 * PndLmdHistogramData.cxx
 *
 *  Created on: Mar 26, 2014
 *      Author: steve
 */

#include "PndLmdHistogramData.h"

ClassImp(PndLmdHistogramData)

PndLmdHistogramData::PndLmdHistogramData() :
		hist_1d(0), hist_2d(0) {
}

PndLmdHistogramData::PndLmdHistogramData(
		const PndLmdHistogramData &lmd_hist_data_) :
		PndLmdAbstractData(lmd_hist_data_), hist_1d(0), hist_2d(0), fit_storage(
				lmd_hist_data_.fit_storage) {
	if (lmd_hist_data_.getPrimaryDimension().is_active) {
		hist_1d = new TH1D(*lmd_hist_data_.get1DHistogram());
		hist_1d->SetDirectory(0);
	}
	if (lmd_hist_data_.getSecondaryDimension().is_active) {
		hist_2d = new TH2D(*lmd_hist_data_.get2DHistogram());
		hist_2d->SetDirectory(0);
	}
}

PndLmdHistogramData::~PndLmdHistogramData() {
	if (hist_1d)
		delete hist_1d;
	if (hist_2d)
		delete hist_2d;
}

void PndLmdHistogramData::init1DData() {
	// 1d histograms
	if (hist_1d)
		delete hist_1d;
	hist_1d = new TH1D("hist1d", "", primary_dimension.bins,
			primary_dimension.dimension_range.getRangeLow(),
			primary_dimension.dimension_range.getRangeHigh());
	hist_1d->SetDirectory(0);

	hist_1d->Sumw2();
}

void PndLmdHistogramData::init2DData() {
	if (hist_2d)
		delete hist_2d;
	hist_2d = new TH2D("hist2d", "", primary_dimension.bins,
			primary_dimension.dimension_range.getRangeLow(),
			primary_dimension.dimension_range.getRangeHigh(),
			secondary_dimension.bins,
			secondary_dimension.dimension_range.getRangeLow(),
			secondary_dimension.dimension_range.getRangeHigh());
	hist_2d->SetDirectory(0);

	hist_2d->Sumw2();
}

/*void PndLmdHistogramData::cloneData(const PndLmdAbstractData &lmd_abs_data) {
 const PndLmdHistogramData * lmd_data =
 dynamic_cast<const PndLmdHistogramData*>(&lmd_abs_data);
 if (lmd_data) {
 hist_1d = new TH1D(*lmd_data->get1DHistogram());
 hist_1d->SetDirectory(0);
 if (getSecondaryDimension().is_active) {
 hist_2d = new TH2D(*lmd_data->get2DHistogram());
 hist_2d->SetDirectory(0);
 }
 }
 }*/

void PndLmdHistogramData::add(const PndLmdAbstractData &lmd_abs_data_addition) {
	const PndLmdHistogramData * lmd_data_addition =
			dynamic_cast<const PndLmdHistogramData*>(&lmd_abs_data_addition);
	if (lmd_data_addition) {
		if (getPrimaryDimension().dimension_range
				== lmd_data_addition->getPrimaryDimension().dimension_range) {
			setNumEvents(getNumEvents() + lmd_data_addition->getNumEvents());
			hist_1d->Add(lmd_data_addition->get1DHistogram());
			if (getSecondaryDimension().is_active) {
				if (getSecondaryDimension().dimension_range
						== lmd_data_addition->getSecondaryDimension().dimension_range) {
					hist_2d->Add(lmd_data_addition->get2DHistogram());
				}
			}
		}
	}
}

void PndLmdHistogramData::addData(double primary_value,
		double secondary_value) {
	hist_1d->Fill(primary_value);
	if (secondary_dimension.is_active) {
		hist_2d->Fill(primary_value, secondary_value);
	}
}

TH1D * PndLmdHistogramData::get1DHistogram() const {
	return hist_1d;
}

TH2D * PndLmdHistogramData::get2DHistogram() const {
	return hist_2d;
}

const map<PndLmdFitOptions, ModelFitResult>& PndLmdHistogramData::getFitResults() const {
	return fit_storage.getFitResults();
}

ModelFitResult PndLmdHistogramData::getFitResult(
		const PndLmdFitOptions &fit_options) const {
	return fit_storage.getFitResult(fit_options);
}

void PndLmdHistogramData::addFitResult(const PndLmdFitOptions &fit_options,
		const ModelFitResult &fit_result_) {
	fit_storage.addFitResult(fit_options, fit_result_);
}

PndLmdHistogramData& PndLmdHistogramData::operator=(
		const PndLmdHistogramData &lmd_hist_data) {
	PndLmdAbstractData::operator=(lmd_hist_data);
	fit_storage = lmd_hist_data.fit_storage;
	if (lmd_hist_data.hist_1d) {
		hist_1d = new TH1D(*(lmd_hist_data.hist_1d));
		hist_1d->SetDirectory(0);
	} else
		hist_1d = 0;
	if (lmd_hist_data.hist_2d) {
		hist_2d = new TH2D(*(lmd_hist_data.hist_2d));
		hist_2d->SetDirectory(0);
	} else
		hist_2d = 0;

	return (*this);
}
