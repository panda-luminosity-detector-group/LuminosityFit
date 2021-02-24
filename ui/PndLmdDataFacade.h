/*
 * PndLmdDataFacade.h
 *
 *  Created on: Aug 26, 2013
 *      Author: steve
 */

#ifndef PNDLMDDATAFACADE_H_
#define PNDLMDDATAFACADE_H_

#include "LumiFitStructs.h"
#include "PndLmdComparisonStructs.h"

#include "PndLmdRuntimeConfiguration.h"

#include "data/PndLmdAbstractData.h"
#include "data/PndLmdMapData.h"
#include "data/PndLmdSeperateDataReader.h"
#include "data/PndLmdCombinedDataReader.h"

#include <vector>

#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations

#include "TFile.h"
#include "TKey.h"

class PndLmdAngularData;
class PndLmdAcceptance;
class PndLmdDataReader;

/**
 * Class providing a simplified UI for constructing, reading and filling lmd
 * data objects. This class should be used primarily used by the standard user.
 */
class PndLmdDataFacade {
  typedef std::vector<std::vector<LumiFit::LmdDimension> > LmdDimensionCombinations;

  const PndLmdRuntimeConfiguration& lmd_runtime_config;

  std::vector<PndLmdMapData> lmd_map_data;
  std::vector<PndLmdAngularData> lmd_angular_data;
  std::vector<PndLmdHistogramData> lmd_vertex_data;
  std::vector<PndLmdHistogramData> lmd_hist_data;
  std::vector<PndLmdAcceptance> lmd_acceptances;

  LmdDimensionCombinations selection_combinations;

  LmdDimensionCombinations extendSelectionsMapByDimensionBundle(
      LmdDimensionCombinations &current_selections_map,
      const LumiFit::LmdDimension &selection_dimension_bundle) const;

  void initializeData(PndLmdAbstractData &data) const;

  LumiFit::LmdDimension constructDimensionFromConfig(
      const boost::property_tree::ptree &pt) const;

  void updateDimensionTypeInfoFromConfig(LumiFit::LmdDimension& dimension,
      const boost::property_tree::ptree &pt) const;

public:
  // data dimension templates
  /*LumiFit::LmdDimension primary_dimension_template;
   LumiFit::LmdDimension secondary_dimension_template;

   // selection dimension bundle vector
   std::vector<LumiFit::LmdDimension> selection_dimension_bundles;

   double current_reference_luminosity_per_event;

   LumiFit::LmdSimIPParameters current_simulation_ip_parameters;*/

  PndLmdDataFacade();
  virtual ~PndLmdDataFacade();

  // low level data creation functions
  void addMultipleInstancesBasedOnSelections(const PndLmdAngularData & data);
  void addMultipleInstancesBasedOnSelections(const PndLmdAcceptance & data);
  void addMultipleInstancesBasedOnSelections(const PndLmdHistogramData & data);

  // high level data creation functions
  void createAngularData();
  void createEfficiencies();
  void createResolutions();
  void createVertexData();

  void adjustDimensionDensity(LumiFit::LmdDimension &dimension,
      const LumiFit::LmdDimension &reference_dimension, bool uneven_bin_count=false) const;

  void applyAutomaticResolutionDimensionRange(
      LumiFit::LmdDimension &dimension) const;

  // selection dimension functions
  void applySelections();
  void appendSelectionDimensions(
      const std::vector<LumiFit::LmdDimension> &selection_dimensions);
  void createSelectionDimensionCombinations(
      const std::vector<LumiFit::LmdDimension>& selection_dimension_bundle);
  void clearSelectionDimensionMap();

  // standard user functions
  void createAndFillDataBundles(const std::string &data_types, PndLmdDataReader &data_reader);
  void createDataBundles(const std::string &data_types);
  void fillCreatedData(PndLmdDataReader &data_reader);
  void saveDataToFiles();
  void cleanup();

  std::vector<PndLmdAngularData> getElasticData() const;
  std::vector<PndLmdAcceptance> getAcceptanceData() const;
  std::vector<PndLmdHistogramData> getResolutionData() const;
  std::vector<PndLmdHistogramData> getVertexData() const;
  std::vector<PndLmdMapData> getMapData() const;
  std::vector<std::string> findFiles(const boost::filesystem::path & dir_path,
      const std::string & file_name) const;
  std::vector<std::string> findFile(const boost::filesystem::path & dir_path,
      const std::string &dir_pattern, const std::string & file_name) const;

  std::vector<PndLmdAcceptance> getMatchingAcceptances(
      const std::vector<PndLmdAcceptance> &acceptance_pool,
      const PndLmdAngularData &lmd_data) const;
  std::vector<PndLmdHistogramData> getMatchingResolutions(
      const std::set<PndLmdHistogramData> &resolution_pool,
      const PndLmdAngularData &lmd_data) const;

  // lmd data object read functions
  std::vector<std::string> findFilesByName(
      const boost::filesystem::path &top_dir_path_to_search,
      const std::string dir_name_filter, const std::string file_name);

  template<class T> std::vector<T> getDataFromFile(TFile& f) const {
    std::vector<T> lmd_data_vec;

    unsigned int counter = 0;

    TIter next(f.GetListOfKeys());
    TKey *key;
    while ((key = (TKey*) next())) {

      T* data;
      f.GetObject(key->GetName(), data);
      if (data) {
        counter++;
        lmd_data_vec.push_back(*data);
        delete data; // this delete is crucial! otherwise we have a memory leak!
      }
    }
    //std::cout << "Found " << counter << " objects!" << std::endl;
    return lmd_data_vec;
  }

  template<class T> std::vector<T> filterData(const std::vector<T> &all_data,
      LumiFit::Comparisons::AbstractLmdDataFilter &filter) const {
    std::vector<T> lmd_data_vec;

    typename std::vector<T>::const_iterator data_instance_iter;
    for (data_instance_iter = all_data.begin();
        data_instance_iter != all_data.end(); ++data_instance_iter) {
      PndLmdAbstractData *lmd_abs_data =
          (PndLmdAbstractData*) &(*data_instance_iter);
      if (lmd_abs_data != 0) {
        if (filter.check(*lmd_abs_data))
          lmd_data_vec.push_back(*data_instance_iter);
      }
    }

    return lmd_data_vec;
  }

  template<class T> std::vector<T> filterData(const std::set<T> &all_data,
      LumiFit::Comparisons::AbstractLmdDataFilter &filter) const {
    std::vector<T> lmd_data_vec;

    typename std::set<T>::const_iterator data_instance_iter;
    for (data_instance_iter = all_data.begin();
        data_instance_iter != all_data.end(); ++data_instance_iter) {
      PndLmdAbstractData *lmd_abs_data =
          (PndLmdAbstractData*) &(*data_instance_iter);
      if (lmd_abs_data != 0) {
        if (filter.check(*lmd_abs_data))
          lmd_data_vec.push_back(*data_instance_iter);
      }
    }

    return lmd_data_vec;
  }

  /*std::map<LumiFit::LmdSimIPParameters,
   std::map<LumiFit::LmdSimIPParameters,
   std::map<LumiFit::LmdDimensionType, std::vector<PndLmdHistogramData> > > > clusterVertexData(
   std::vector<PndLmdHistogramData> &lmd_vertex_vec);*/

};

template<> std::vector<PndLmdMapData> PndLmdDataFacade::getDataFromFile(TFile& f) const;

#endif /* PNDLMDDATAFACADE_H_ */
