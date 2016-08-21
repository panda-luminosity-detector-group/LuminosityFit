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
#include "PndLmdMapData.h"

class PndLmdElasticDataBundle: public PndLmdAngularData {
	friend class PndLmdFitDataBundle;

	std::vector<unsigned int> used_acceptance_indices;
	//std::vector<std::pair<unsigned int, unsigned int> > used_resolutions_index_ranges;
	std::vector<unsigned int> used_resolution_map_indices;

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

	/*const std::vector<std::pair<unsigned int, unsigned int> >& getUsedResolutionsIndexRanges() const {
		return used_resolutions_index_ranges;
	}*/

  const std::vector<unsigned int>& getUsedResolutionIndices() const {
    return used_resolution_map_indices;
  }

	ClassDef(PndLmdElasticDataBundle, 1);
};

class PndLmdFitDataBundle: public TObject {
	std::vector<PndLmdElasticDataBundle> elastic_data_bundles;
	std::vector<PndLmdAcceptance> used_acceptances_pool;
	std::vector<PndLmdMapData> used_resolutions_pool;

	PndLmdElasticDataBundle current_elastic_data_bundle;

	std::vector<std::pair<unsigned int, unsigned int> > convertToIndexRanges(
			const std::vector<unsigned int> &single_positions) const;

	unsigned int addAcceptanceToPool(
			const PndLmdAcceptance &new_acceptance);
	//std::vector<std::pair<unsigned int, unsigned int> > addResolutionsToPool(
	//		const std::vector<PndLmdMapData>& new_resolutions);
	unsigned int addResolutionToPool(
	      const PndLmdMapData& new_resolution);

public:
	PndLmdFitDataBundle();
	virtual ~PndLmdFitDataBundle();

	const std::vector<PndLmdElasticDataBundle>& getElasticDataBundles() const;
	const std::vector<PndLmdAcceptance>& getUsedAcceptancesPool() const;
	const std::vector<PndLmdMapData>& getUsedResolutionsPool() const;

	void addFittedElasticData(
	    const PndLmdAngularData &elastic_data);
	void attachAcceptanceToCurrentData(const PndLmdAcceptance &acceptance);
	void attachResolutionMapDataToCurrentData(const PndLmdMapData &resolution);
	void addCurrentDataBundleToList();

	void saveDataBundleToRootFile(const std::string &file_url) const;

ClassDef(PndLmdFitDataBundle, 2);
};

#endif /* PNDLMDFITDATABUNDLE_H_ */
