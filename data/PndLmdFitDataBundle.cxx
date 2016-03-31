/*
 * PndLmdFitDataBundle.cxx
 *
 *  Created on: Jun 2, 2015
 *      Author: steve
 */

#include "PndLmdFitDataBundle.h"

#include <iostream>

#include "TFile.h"

ClassImp(PndLmdFitDataBundle)

PndLmdFitDataBundle::PndLmdFitDataBundle() {
  // TODO Auto-generated constructor stub

}

PndLmdFitDataBundle::~PndLmdFitDataBundle() {
  // TODO Auto-generated destructor stub
}

const std::vector<PndLmdElasticDataBundle>& PndLmdFitDataBundle::getElasticDataBundles() const {
  return elastic_data_bundles;
}

const std::vector<PndLmdAcceptance>& PndLmdFitDataBundle::getUsedAcceptancesPool() const {
  return used_acceptances_pool;
}

const std::vector<PndLmdHistogramData>& PndLmdFitDataBundle::getUsedResolutionsPool() const {
  return used_resolutions_pool;
}

std::vector<std::pair<unsigned int, unsigned int> > PndLmdFitDataBundle::convertToIndexRanges(
    const std::vector<unsigned int> &single_positions) const {

  std::vector<std::pair<unsigned int, unsigned int> > position_ranges;

  unsigned int last_position(0);
  std::pair<unsigned int, unsigned int> current_range;

  for (unsigned int i = 0; i < single_positions.size(); ++i) {
    if (0 == i)
      current_range.first = single_positions[i];

    else {
      if (single_positions[i] != last_position + 1) {
        current_range.second = last_position;
        position_ranges.push_back(current_range);
        current_range.first = single_positions[i];
      }
    }

    if (single_positions.size() - 1 == i) {
      current_range.second = single_positions[i];
      position_ranges.push_back(current_range);
    }

    last_position = single_positions[i];
  }

  return position_ranges;
}

std::vector<unsigned int> PndLmdFitDataBundle::addAcceptancesToPool(
    const std::vector<PndLmdAcceptance> &new_acceptances) {
  std::vector<unsigned int> positions;
  positions.reserve(new_acceptances.size());

  for (unsigned int new_acc_index = 0; new_acc_index < new_acceptances.size();
      ++new_acc_index) {

    // first check if this acceptance already exists
    std::vector<PndLmdAcceptance>::iterator search_result_iter = std::find(
        used_acceptances_pool.begin(), used_acceptances_pool.end(),
        new_acceptances[new_acc_index]);
    unsigned int position(used_acceptances_pool.size());
    if (search_result_iter == used_acceptances_pool.end()) {
      // if not add and return position is the last element
      used_acceptances_pool.push_back(new_acceptances[new_acc_index]);
    } else {
      // if so just return the found position index
      position = search_result_iter - used_acceptances_pool.begin();
    }
    positions.push_back(position);
  }
  return positions;
}

std::vector<std::pair<unsigned int, unsigned int> > PndLmdFitDataBundle::addResolutionsToPool(
    const std::vector<PndLmdHistogramData>& new_resolutions) {

  std::vector<std::pair<unsigned int, unsigned int> > positions;
  std::vector<unsigned int> single_positions;
  single_positions.reserve(new_resolutions.size());

  for (unsigned int new_res_index = 0; new_res_index < new_resolutions.size();
      ++new_res_index) {
    // first check if this acceptance already exists
    std::vector<PndLmdHistogramData>::iterator search_result_iter = std::find(
        used_resolutions_pool.begin(), used_resolutions_pool.end(),
        new_resolutions[new_res_index]);
    if (search_result_iter == used_resolutions_pool.end()) {
      // if not add and return position is the last element
      used_resolutions_pool.push_back(new_resolutions[new_res_index]);
      single_positions.push_back(used_resolutions_pool.size() - 1);
    } else {
      // if so just return the found position index
      single_positions.push_back(
          search_result_iter - used_resolutions_pool.begin());
    }
  }

  // sort the vector
  std::sort(single_positions.begin(), single_positions.end());
  return convertToIndexRanges(single_positions);
}

void PndLmdFitDataBundle::addFittedElasticData(
    const PndLmdAngularData &elastic_data,
    const std::vector<PndLmdAcceptance> &acceptances,
    const std::vector<PndLmdHistogramData> &resolutions) {

  PndLmdElasticDataBundle elastic_data_bundle(elastic_data);
  elastic_data_bundle.used_acceptance_indices = addAcceptancesToPool(
      acceptances);
  elastic_data_bundle.used_resolutions_index_ranges = addResolutionsToPool(
      resolutions);

  elastic_data_bundles.push_back(elastic_data_bundle);
}

void PndLmdFitDataBundle::saveDataBundleToRootFile(
    const std::string &file_url) const {
  // create output file
  TFile f(file_url.c_str(), "RECREATE");

  std::cout << "Saving data...." << std::endl;

  f.cd();
  this->Write("lmd_elastic_data_bundle");
  f.Close();
  std::cout << std::endl << "Successfully saved data!" << std::endl;
}
