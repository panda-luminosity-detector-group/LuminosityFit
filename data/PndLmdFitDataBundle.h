/*
 * PndLmdFitDataBundle.h
 *
 *  Created on: Jun 2, 2015
 *      Author: steve
 */

#ifndef PNDLMDFITDATABUNDLE_H_
#define PNDLMDFITDATABUNDLE_H_

#include "PndLmdAngularData.h"
#include "PndLmdAcceptance.h"

class PndLmdElasticDataBundle: public PndLmdAngularData {
	friend class PndLmdFitDataBundle;

	std::vector<unsigned int> used_acceptance_indices;
	std::vector<std::pair<unsigned int, unsigned int> > used_resolutions_index_ranges;

public:
	PndLmdElasticDataBundle() {
	}
	PndLmdElasticDataBundle(const PndLmdAngularData& elastic_data_) :
			PndLmdAngularData(elastic_data_) {
	}
	virtual ~PndLmdElasticDataBundle() {
	}

	const std::vector<unsigned int>& getUsedAcceptanceIndices() const {
		return used_acceptance_indices;
	}

	const std::vector<std::pair<unsigned int, unsigned int> >& getUsedResolutionsIndexRanges() const {
		return used_resolutions_index_ranges;
	}

	ClassDef(PndLmdElasticDataBundle, 1);
};

class PndLmdFitDataBundle: public TObject {
	std::vector<PndLmdElasticDataBundle> elastic_data_bundles;
	std::vector<PndLmdAcceptance> used_acceptances_pool;
	std::vector<PndLmdHistogramData> used_resolutions_pool;

	std::vector<std::pair<unsigned int, unsigned int> > convertToIndexRanges(
			const std::vector<unsigned int> &single_positions) const;

	std::vector<unsigned int> addAcceptancesToPool(
			const std::vector<PndLmdAcceptance> &new_acceptances);
	std::vector<std::pair<unsigned int, unsigned int> > addResolutionsToPool(
			const std::vector<PndLmdHistogramData>& new_resolutions);

public:
	PndLmdFitDataBundle();
	virtual ~PndLmdFitDataBundle();

	const std::vector<PndLmdElasticDataBundle>& getElasticDataBundles() const;
	const std::vector<PndLmdAcceptance>& getUsedAcceptancesPool() const;
	const std::vector<PndLmdHistogramData>& getUsedResolutionsPool() const;

	void addFittedElasticData(const PndLmdAngularData &elastic_data,
			const std::vector<PndLmdAcceptance> &acceptances,
			const std::vector<PndLmdHistogramData> &resolutions);

	void saveDataBundleToRootFile(const std::string &file_url) const;

ClassDef(PndLmdFitDataBundle, 1);
};

#endif /* PNDLMDFITDATABUNDLE_H_ */
