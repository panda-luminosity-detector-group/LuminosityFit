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
  
}

PndLmdFitDataBundle::~PndLmdFitDataBundle() {
  
}

const std::vector<PndLmdElasticDataBundle> &
PndLmdFitDataBundle::getElasticDataBundles() const {
  return elastic_data_bundles;
}

const std::vector<PndLmdAcceptance> &
PndLmdFitDataBundle::getUsedAcceptancesPool() const {
  return used_acceptances_pool;
}

const std::vector<PndLmdMapData> &
PndLmdFitDataBundle::getUsedResolutionsPool() const {
  return used_resolutions_pool;
}

std::vector<std::pair<unsigned int, unsigned int>>
PndLmdFitDataBundle::convertToIndexRanges(
    const std::vector<unsigned int> &single_positions) const {

  std::vector<std::pair<unsigned int, unsigned int>> position_ranges;

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

unsigned int PndLmdFitDataBundle::addAcceptanceToPool(
    const PndLmdAcceptance &new_acceptance) {

  // first check if this acceptance already exists
  std::vector<PndLmdAcceptance>::iterator search_result_iter =
      std::find(used_acceptances_pool.begin(), used_acceptances_pool.end(),
                new_acceptance);
  unsigned int position(used_acceptances_pool.size());
  if (search_result_iter == used_acceptances_pool.end()) {
    // if not add and return position is the last element
    std::cout << "adding resolution to pool...\n";
    used_acceptances_pool.push_back(new_acceptance);
  } else {
    // if so just return the found position index
    position = search_result_iter - used_acceptances_pool.begin();
  }

  return position;
}

unsigned int
PndLmdFitDataBundle::addResolutionToPool(const PndLmdMapData &new_resolution) {

  // first check if this acceptance already exists
  std::vector<PndLmdMapData>::iterator search_result_iter =
      std::find(used_resolutions_pool.begin(), used_resolutions_pool.end(),
                new_resolution);
  unsigned int position(used_resolutions_pool.size());
  if (search_result_iter == used_resolutions_pool.end()) {
    // if not add and return position is the last element
    used_resolutions_pool.push_back(new_resolution);
    std::cout << "adding resolution to pool...\n";
    // used_resolutions_pool[used_resolutions_pool.size()-1].clearMap();
  } else {
    // if so just return the found position index
    position = search_result_iter - used_resolutions_pool.begin();
  }

  return position;
  // sort the vector
  // std::sort(single_positions.begin(), single_positions.end());
  // return convertToIndexRanges(single_positions);
}

void PndLmdFitDataBundle::addFittedElasticData(
    const PndLmdAngularData &elastic_data) {
  current_elastic_data_bundle = PndLmdElasticDataBundle(elastic_data);
}
void PndLmdFitDataBundle::attachAcceptanceToCurrentData(
    const PndLmdAcceptance &acceptance) {
  current_elastic_data_bundle.used_acceptance_indices.push_back(
      addAcceptanceToPool(acceptance));
}
void PndLmdFitDataBundle::attachResolutionMapDataToCurrentData(
    const PndLmdMapData &resolution) {
  current_elastic_data_bundle.used_resolution_map_indices.push_back(
      addResolutionToPool(resolution));
}
void PndLmdFitDataBundle::addCurrentDataBundleToList() {
  elastic_data_bundles.push_back(current_elastic_data_bundle);
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

void PndLmdFitDataBundle::printInfo() const {
  std::cout << "available acceptances: " << getUsedAcceptancesPool().size()
            << std::endl;
  std::cout << "available resolutions: " << getUsedResolutionsPool().size()
            << std::endl;
  for (auto const &elastic_data : getElasticDataBundles()) {
    std::cout << "elastic data bundle: "
              << elastic_data.getPrimaryDimension().createDimensionLabel()
              << std::endl;
    std::cout << "used acceptances: "
              << elastic_data.getUsedAcceptanceIndices().size() << std::endl;
    std::cout << "used resolutions: "
              << elastic_data.getUsedResolutionIndices().size() << std::endl;
  }
}

const PndLmdElasticDataBundle & PndLmdFitDataBundle::getCurrentElasticBundle() const{
  return current_elastic_data_bundle;
}