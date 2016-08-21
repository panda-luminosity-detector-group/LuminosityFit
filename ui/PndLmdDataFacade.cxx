/*
 * PndLmdDataFacade.cxx
 *
 *  Created on: Aug 26, 2013
 *      Author: steve
 */

#include "PndLmdDataFacade.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"
#include "data/PndLmdMapData.h"
#include "fit/data/DataStructs.h"

#include <sstream>
#include <fstream>
#include <iostream>

#include "boost/regex.hpp"
#include "boost/foreach.hpp"

using boost::property_tree::ptree;
using std::vector;
using std::string;
using boost::filesystem::path;
using boost::filesystem::directory_iterator;

PndLmdDataFacade::PndLmdDataFacade() :
    lmd_runtime_config(PndLmdRuntimeConfiguration::Instance()) {
}

PndLmdDataFacade::~PndLmdDataFacade() {
  // TODO Auto-generated destructor stub
}

void PndLmdDataFacade::addFileList(PndLmdDataReader& data_reader,
    const std::string filelist) {
  // scan which data reader has to create this object
  std::ifstream input(filelist.c_str());
  std::string line;

  while (std::getline(input, line)) {
    std::cout << line << std::endl;
    data_reader.addFilePath(line);
  }
}

void PndLmdDataFacade::initializeData(PndLmdAbstractData & data) const {
  data.setNumEvents(lmd_runtime_config.getNumEvents());
  data.setLabMomentum(lmd_runtime_config.getMomentum());

  data.setSimulationParameters(
      PndLmdRuntimeConfiguration::Instance().getSimulationParameters());
}

void PndLmdDataFacade::addMultipleInstancesBasedOnSelections(
    const PndLmdAngularData & data) {
  if (selection_combinations.size() > 0) {
    LmdDimensionCombinations::iterator selection_combinations_iter =
        selection_combinations.begin();
    while (selection_combinations_iter != selection_combinations.end()) {
      // clone the data object
      PndLmdAngularData data_clone(data);
      // generate name and add selection dimensions
      std::stringstream ss;
      ss << data.getName();
      for (unsigned int i = 0; i < selection_combinations_iter->size(); i++) {
        ss << "_" << (*selection_combinations_iter)[i].name;
        data_clone.addSelectionDimension((*selection_combinations_iter)[i]);
      }
      data_clone.setName(ss.str());
      lmd_angular_data.push_back(data_clone);
      ++selection_combinations_iter;
    }
  } else
    lmd_angular_data.push_back(data);
}

void PndLmdDataFacade::addMultipleInstancesBasedOnSelections(
    const PndLmdAcceptance & data) {
  if (selection_combinations.size() > 0) {
    LmdDimensionCombinations::iterator selection_combinations_iter =
        selection_combinations.begin();
    while (selection_combinations_iter != selection_combinations.end()) {
      // clone the data object
      PndLmdAcceptance data_clone(data);
      // generate name and add selection dimensions
      std::stringstream ss;
      ss << data.getName();
      for (unsigned int i = 0; i < selection_combinations_iter->size(); i++) {
        ss << "_" << (*selection_combinations_iter)[i].name;
        data_clone.addSelectionDimension((*selection_combinations_iter)[i]);
      }
      data_clone.setName(ss.str());
      lmd_acceptances.push_back(data_clone);
      ++selection_combinations_iter;
    }
  } else
    lmd_acceptances.push_back(data);
}

void PndLmdDataFacade::addMultipleInstancesBasedOnSelections(
    const PndLmdHistogramData & data) {
  std::vector<PndLmdHistogramData> *storage_vector = &lmd_hist_data;

  if (LumiFit::X == data.getPrimaryDimension().dimension_options.dimension_type
      || LumiFit::Y
          == data.getPrimaryDimension().dimension_options.dimension_type
      || LumiFit::Z
          == data.getPrimaryDimension().dimension_options.dimension_type) {
    storage_vector = &lmd_vertex_data;
  }

  if (selection_combinations.size() > 0) {
    LmdDimensionCombinations::iterator selection_combinations_iter =
        selection_combinations.begin();
    while (selection_combinations_iter != selection_combinations.end()) {
      // clone the data object
      PndLmdHistogramData data_clone(data);
      // generate name and add selection dimensions
      std::stringstream ss;
      ss << data.getName();
      for (unsigned int i = 0; i < selection_combinations_iter->size(); i++) {
        ss << "_" << (*selection_combinations_iter)[i].name;
        data_clone.addSelectionDimension((*selection_combinations_iter)[i]);
      }
      data_clone.setName(ss.str());
      storage_vector->push_back(data_clone);
      ++selection_combinations_iter;
    }
  } else
    storage_vector->push_back(data);
}

LumiFit::LmdDimension PndLmdDataFacade::constructDimensionFromConfig(
    const boost::property_tree::ptree &pt) const {
  LumiFit::LmdDimension dimension;

  boost::optional<bool> active_option = pt.get_optional<bool>("is_active");
  if (active_option.is_initialized())
    dimension.is_active = active_option.get();

  boost::optional<std::string> dim_type_option = pt.get_optional<std::string>(
      "dimension_type");
  if (dim_type_option.is_initialized())
    dimension.dimension_options.dimension_type =
        LumiFit::StringToDimensionType.at(dim_type_option.get());
  boost::optional<std::string> track_param_type_option = pt.get_optional<
      std::string>("track_param_type");
  if (track_param_type_option.is_initialized())
    dimension.dimension_options.track_param_type =
        LumiFit::StringToTrackParamType.at(track_param_type_option.get());

  dimension.bins = pt.get<int>("bins");
  dimension.dimension_range.setRangeLow(pt.get<double>("range_low"));
  dimension.dimension_range.setRangeHigh(pt.get<double>("range_high"));
  dimension.calculateBinSize();

  boost::optional<std::string> name_option = pt.get_optional<std::string>(
      "name");
  if (name_option.is_initialized())
    dimension.name = name_option.get();

  return dimension;
}

void PndLmdDataFacade::updateDimensionTypeInfoFromConfig(
    LumiFit::LmdDimension& dimension,
    const boost::property_tree::ptree &pt) const {

  if (dimension.is_active) {
    boost::optional<std::string> dim_type_option = pt.get_optional<std::string>(
        "dimension_type");
    if (dim_type_option.is_initialized())
      dimension.dimension_options.dimension_type =
          LumiFit::StringToDimensionType.at(dim_type_option.get());
    boost::optional<std::string> track_param_type_option = pt.get_optional<
        std::string>("track_param_type");
    if (track_param_type_option.is_initialized())
      dimension.dimension_options.track_param_type =
          LumiFit::StringToTrackParamType.at(track_param_type_option.get());
  }
}

void PndLmdDataFacade::createAngularData() {
  PndLmdAngularData data_template;
  LumiFit::LmdDimension primary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.primary_dimension"));
  LumiFit::LmdDimension secondary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.secondary_dimension"));

  initializeData(data_template);
  data_template.setReferenceLuminosityPerEvent(
      1.0 / lmd_runtime_config.getTotalElasticCrossSection());

  applySelections();

  /*LumiFit::LmdDimensionRange theta_dimension_range =
   primary_dimension_template.dimension_range;

   PndLmdLumiHelper helper;
   LumiFit::LmdDimensionRange t_dimension_range;
   t_dimension_range.setRangeLow(
   helper.getMomentumTransferFromTheta(lab_momentum,
   primary_dimension_template.dimension_range.getRangeLow()));
   t_dimension_range.setRangeHigh(
   helper.getMomentumTransferFromTheta(lab_momentum,
   primary_dimension_template.dimension_range.getRangeHigh()));

   primary_dimension_template.dimension_options.dimension_type = LumiFit::T;
   primary_dimension_template.dimension_options.track_type = LumiFit::MC;
   primary_dimension_template.dimension_range = t_dimension_range;
   createData1D("mc_t", num_events);*/

  primary_dimension.dimension_options.track_type = LumiFit::MC;
  secondary_dimension.dimension_options.track_type = LumiFit::MC;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("mc_th");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.track_type = LumiFit::MC_ACC;
  secondary_dimension.dimension_options.track_type = LumiFit::MC_ACC;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("mc_acc");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.track_type = LumiFit::RECO;
  secondary_dimension.dimension_options.track_type = LumiFit::RECO;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("reco");
  addMultipleInstancesBasedOnSelections(data_template);

  if (selection_combinations.size() > 0) {
    selection_combinations.clear();
    addMultipleInstancesBasedOnSelections(data_template);
  }
}

void PndLmdDataFacade::createEfficiencies() {
  PndLmdAcceptance data_template;

  LumiFit::LmdDimension primary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "efficiency.primary_dimension"));
  updateDimensionTypeInfoFromConfig(primary_dimension,
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.primary_dimension"));
  adjustDimensionDensity(primary_dimension,
      constructDimensionFromConfig(
          lmd_runtime_config.getDataConfigTree().get_child(
              "general_data.primary_dimension")));

  LumiFit::LmdDimension secondary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "efficiency.secondary_dimension"));
  updateDimensionTypeInfoFromConfig(secondary_dimension,
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.secondary_dimension"));
  adjustDimensionDensity(secondary_dimension,
      constructDimensionFromConfig(
          lmd_runtime_config.getDataConfigTree().get_child(
              "general_data.secondary_dimension")));

  initializeData(data_template);

  applySelections();

  primary_dimension.dimension_options.track_type = LumiFit::MC;
  secondary_dimension.dimension_options.track_type = LumiFit::MC;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("lmd_acceptance");
  addMultipleInstancesBasedOnSelections(data_template);
}

void PndLmdDataFacade::createResolutions() {
  PndLmdMapData data_template;
  LumiFit::LmdDimension primary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.primary_dimension"));
  LumiFit::LmdDimension secondary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.secondary_dimension"));

  initializeData(data_template);

  primary_dimension.dimension_options.track_type = LumiFit::RECO;
  secondary_dimension.dimension_options.track_type = LumiFit::RECO;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("res");
  lmd_map_data.push_back(data_template);

  PndLmdHistogramData res_data_template;
  initializeData(res_data_template);

  primary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "resolution.primary_dimension"));
  updateDimensionTypeInfoFromConfig(primary_dimension,
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.primary_dimension"));

  secondary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "resolution.secondary_dimension"));
  updateDimensionTypeInfoFromConfig(secondary_dimension,
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.secondary_dimension"));

  primary_dimension.dimension_options.track_type = LumiFit::DIFF_RECO_MC;
  secondary_dimension.dimension_options.track_type = LumiFit::DIFF_RECO_MC;

  boost::optional<bool> use_automatic_range_prim_dim =
      lmd_runtime_config.getDataConfigTree().get_optional<bool>(
          "resolution.primary_dimension.use_automatic_range");
  boost::optional<bool> use_automatic_range_sec_dim =
      lmd_runtime_config.getDataConfigTree().get_optional<bool>(
          "resolution.secondary_dimension.use_automatic_range");

  if (use_automatic_range_prim_dim.is_initialized()
      && use_automatic_range_prim_dim.get())
    applyAutomaticResolutionDimensionRange(primary_dimension);
  if (use_automatic_range_sec_dim.is_initialized()
      && use_automatic_range_sec_dim.get())
    applyAutomaticResolutionDimensionRange(secondary_dimension);

  /*adjustDimensionDensity(primary_dimension,
   constructDimensionFromConfig(
   lmd_runtime_config.getDataConfigTree().get_child(
   "general_data.primary_dimension")), true);
   adjustDimensionDensity(secondary_dimension,
   constructDimensionFromConfig(
   lmd_runtime_config.getDataConfigTree().get_child(
   "general_data.secondary_dimension")), true);*/

  res_data_template.setPrimaryDimension(primary_dimension);
  res_data_template.setSecondaryDimension(secondary_dimension);

  res_data_template.setName("res_hist");

  lmd_hist_data.push_back(res_data_template);

  /*std::vector<LumiFit::LmdDimension> selection_dimension_bundles;

   LumiFit::LmdDimension primary_selection_dimension_bundle =
   constructDimensionFromConfig(
   lmd_runtime_config.getDataConfigTree().get_child(
   "resolution.primary_selection_dimension"));
   updateDimensionTypeInfoFromConfig(primary_selection_dimension_bundle,
   lmd_runtime_config.getDataConfigTree().get_child(
   "general_data.primary_dimension"));
   adjustDimensionDensity(primary_selection_dimension_bundle,
   constructDimensionFromConfig(
   lmd_runtime_config.getDataConfigTree().get_child(
   "general_data.primary_dimension")));

   primary_selection_dimension_bundle.dimension_options.track_type = LumiFit::MC;

   selection_dimension_bundles.push_back(primary_selection_dimension_bundle);

   LumiFit::LmdDimension secondary_selection_dimension_bundle =
   constructDimensionFromConfig(
   lmd_runtime_config.getDataConfigTree().get_child(
   "resolution.primary_selection_dimension"));
   updateDimensionTypeInfoFromConfig(secondary_selection_dimension_bundle,
   lmd_runtime_config.getDataConfigTree().get_child(
   "general_data.secondary_dimension"));
   adjustDimensionDensity(secondary_selection_dimension_bundle,
   constructDimensionFromConfig(
   lmd_runtime_config.getDataConfigTree().get_child(
   "general_data.secondary_dimension")));

   if (secondary_selection_dimension_bundle.is_active) {
   secondary_selection_dimension_bundle.dimension_options.track_type =
   LumiFit::MC;

   selection_dimension_bundles.push_back(secondary_selection_dimension_bundle);
   }

   createSelectionDimensionCombinations(selection_dimension_bundles);

   applySelections();

   addMultipleInstancesBasedOnSelections(data_template);*/
}

void PndLmdDataFacade::adjustDimensionDensity(LumiFit::LmdDimension &dimension,
    const LumiFit::LmdDimension &reference_dimension,
    bool uneven_bin_count) const {
  // make bin size same as reference and increase range to match user range

  double bins = std::ceil(
      dimension.dimension_range.getDimensionLength()
          / reference_dimension.bin_size);
  if (uneven_bin_count && ((unsigned int) bins % 2 == 0))
    ++bins;
  dimension.bins = (unsigned int) bins;

  double range = bins * reference_dimension.bin_size / 2;

  double dimension_mean = dimension.dimension_range.getDimensionMean();
  dimension.dimension_range.setRangeLow(dimension_mean - range);
  dimension.dimension_range.setRangeHigh(dimension_mean + range);

  dimension.bin_size = reference_dimension.bin_size;
}

void PndLmdDataFacade::applyAutomaticResolutionDimensionRange(
    LumiFit::LmdDimension &dimension) const {
  double sigma = 5.0;
// simple momentum dependent resolution parameterization (in mrad)
// exp + const
  double theta_resolution_estimate = 0.001
      * (0.09 + 1.8 * exp(-0.7 * lmd_runtime_config.getMomentum()));
  double theta_plot_range_max = sigma * theta_resolution_estimate;
// this is just guessed from the top formulas above for phi (in rad)
  double phi_resolution_estimate = 0.03
      + 0.6 * exp(-0.7 * lmd_runtime_config.getMomentum());
  double phi_plot_range_max = sigma * phi_resolution_estimate;

  if (dimension.dimension_options.dimension_type == LumiFit::PHI) {
    dimension.dimension_range.setRangeLow(-phi_plot_range_max);
    dimension.dimension_range.setRangeHigh(phi_plot_range_max);
  } else {
    dimension.dimension_range.setRangeLow(-theta_plot_range_max);
    dimension.dimension_range.setRangeHigh(theta_plot_range_max);
  }
}

void PndLmdDataFacade::createVertexData() {
  PndLmdHistogramData data_template;

  LumiFit::LmdDimension primary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "vertex.primary_dimension"));
  updateDimensionTypeInfoFromConfig(primary_dimension,
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.primary_dimension"));

  LumiFit::LmdDimension secondary_dimension = constructDimensionFromConfig(
      lmd_runtime_config.getDataConfigTree().get_child(
          "vertex.secondary_dimension"));
  updateDimensionTypeInfoFromConfig(secondary_dimension,
      lmd_runtime_config.getDataConfigTree().get_child(
          "general_data.secondary_dimension"));

  primary_dimension.dimension_options.track_param_type = LumiFit::IP;
  secondary_dimension.dimension_options.track_param_type = LumiFit::IP;

  initializeData(data_template);

  applySelections();

  // 1d projections first
  secondary_dimension.is_active = false;
  data_template.setSecondaryDimension(secondary_dimension);

  primary_dimension.dimension_options.track_type = LumiFit::MC;
  primary_dimension.dimension_options.dimension_type = LumiFit::X;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setName("mc_x");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.dimension_type = LumiFit::Y;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setName("mc_y");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.dimension_type = LumiFit::Z;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setName("mc_z");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.track_type = LumiFit::RECO;
  primary_dimension.dimension_options.dimension_type = LumiFit::X;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setName("reco_x");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.dimension_type = LumiFit::Y;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setName("reco_y");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.dimension_type = LumiFit::Z;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setName("reco_z");
  addMultipleInstancesBasedOnSelections(data_template);

  // 2d stuff
  secondary_dimension.is_active = true;
  primary_dimension.dimension_options.track_type = LumiFit::MC;
  secondary_dimension.dimension_options.track_type = LumiFit::MC;
  primary_dimension.dimension_options.dimension_type = LumiFit::X;
  secondary_dimension.dimension_options.dimension_type = LumiFit::Y;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("mc_xy");
  addMultipleInstancesBasedOnSelections(data_template);

  primary_dimension.dimension_options.track_type = LumiFit::RECO;
  secondary_dimension.dimension_options.track_type = LumiFit::RECO;
  data_template.setPrimaryDimension(primary_dimension);
  data_template.setSecondaryDimension(secondary_dimension);
  data_template.setName("reco_xy");
  addMultipleInstancesBasedOnSelections(data_template);
}

void PndLmdDataFacade::applySelections() {
  std::vector<LumiFit::LmdDimension> selection_dimensions;

  BOOST_FOREACH(const ptree::value_type & sel_dim,
      lmd_runtime_config.getDataConfigTree().get_child("selections")){
  LumiFit::LmdDimension selection_dimension =
  constructDimensionFromConfig(sel_dim.second);
  selection_dimensions.push_back(selection_dimension);
}

  appendSelectionDimensions(selection_dimensions);
}

void PndLmdDataFacade::clearSelectionDimensionMap() {
  selection_combinations.clear();
}

void PndLmdDataFacade::appendSelectionDimensions(
    const std::vector<LumiFit::LmdDimension> &selection_dimensions) {

  if (selection_combinations.size() == 0) {
    selection_combinations.push_back(selection_dimensions);
  } else {
    LmdDimensionCombinations::iterator current_selection_map_it =
        selection_combinations.begin();
    while (current_selection_map_it != selection_combinations.end()) {
      current_selection_map_it->insert(current_selection_map_it->begin(),
          selection_dimensions.begin(), selection_dimensions.end());
      ++current_selection_map_it;
    }
  }
}

void PndLmdDataFacade::createSelectionDimensionCombinations(
    const std::vector<LumiFit::LmdDimension>& selection_dimension_bundle) {
// create a map of selection dimension lists
// one entry in the map for each combination
  selection_combinations.clear();
  LmdDimensionCombinations temp_selections_combinations;

  LumiFit::LmdDimension selection_dimension;

// for the first selection dimension bundle just fill the map with all different combinations
  for (unsigned int i = 0; i < selection_dimension_bundle.size(); i++) {
    if (0 == i) {
      selection_dimension = selection_dimension_bundle[i];

      selection_dimension.bins = 1;

      for (unsigned int slice_index = 0;
          slice_index < selection_dimension_bundle[i].bins; slice_index++) {
        selection_dimension.dimension_range.setRangeLow(
            selection_dimension_bundle[i].dimension_range.getRangeLow()
                + selection_dimension_bundle[i].bin_size * slice_index);
        selection_dimension.dimension_range.setRangeHigh(
            selection_dimension_bundle[i].dimension_range.getRangeLow()
                + selection_dimension_bundle[i].bin_size * (slice_index + 1));
        selection_dimension.calculateBinSize();
        std::stringstream ss;
        ss << slice_index;
        selection_dimension.name = ss.str();
        std::vector<LumiFit::LmdDimension> selection_dimensions;
        selection_dimensions.push_back(selection_dimension);
        temp_selections_combinations.push_back(selection_dimensions);
      }

    } else {
      temp_selections_combinations = extendSelectionsMapByDimensionBundle(
          temp_selections_combinations, selection_dimension_bundle[i]);
    }
  }
  selection_combinations = temp_selections_combinations;
  std::cout << "successfully created selection dimension combination map!"
      << std::endl;
}

PndLmdDataFacade::LmdDimensionCombinations PndLmdDataFacade::extendSelectionsMapByDimensionBundle(
    LmdDimensionCombinations &current_selection_combinations,
    const LumiFit::LmdDimension &selection_dimension_bundle) const {

  LmdDimensionCombinations new_selections_combinations;
// for the remaining selection dimensions
// (atm its just the secondary, but it could be more in principle):
// loop over them, and for each entry in the map

  LumiFit::LmdDimension selection_dimension = selection_dimension_bundle;

  selection_dimension.bins = 1;

  for (unsigned int slice_index = 0;
      slice_index < selection_dimension_bundle.bins; slice_index++) {
    selection_dimension.dimension_range.setRangeLow(
        selection_dimension_bundle.dimension_range.getRangeLow()
            + selection_dimension_bundle.bin_size * slice_index);
    selection_dimension.dimension_range.setRangeHigh(
        selection_dimension_bundle.dimension_range.getRangeLow()
            + selection_dimension_bundle.bin_size * (slice_index + 1));

    selection_dimension.calculateBinSize();
    // loop over current map and for each entry create a new entry with addition suffix
    LmdDimensionCombinations::iterator current_selection_combination_it =
        current_selection_combinations.begin();
    while (current_selection_combination_it
        != current_selection_combinations.end()) {
      std::stringstream ss;
      ss << slice_index;
      selection_dimension.name = ss.str();

      std::vector<LumiFit::LmdDimension> current_selection_dimensions =
          *current_selection_combination_it;
      current_selection_dimensions.push_back(selection_dimension);
      new_selections_combinations.push_back(current_selection_dimensions);
      ++current_selection_combination_it;
    }
  }

  return new_selections_combinations;
}

void PndLmdDataFacade::createAndFillDataBundles(const std::string &data_types) {
  // create data
  createDataBundles(data_types);

// fill data
  fillCreatedData();

// write to file
  saveDataToFiles();

// clean up
  cleanup();
}

void PndLmdDataFacade::createDataBundles(const std::string &data_types) {
  if (data_types.find("a") != std::string::npos) {
    // ---- dpm or elastic data ---- //
    createAngularData();
  }
  if (data_types.find("e") != std::string::npos) {
    // ---- acceptance or box gen data ---- //
    createEfficiencies();
  }
  if (data_types.find("r") != std::string::npos) {
    // ---- create lmd resolution objects from box gen data ---- //
    createResolutions();
  }
  if (data_types.find("v") != std::string::npos) {
    // ---- create lmd vertex objects from dpm or elastic data ---- //
    createVertexData();
  }
}

void PndLmdDataFacade::fillCreatedData() {
  PndLmdCombinedDataReader data_reader;

// add input directory to data facade
  if (boost::filesystem::is_empty(lmd_runtime_config.getRawDataFilelistPath()))
    data_reader.addFilePath(
        lmd_runtime_config.getRawDataDirectory().string()
            + "/Lumi_TrksQA*.root");
  else {
    addFileList(data_reader,
        lmd_runtime_config.getRawDataFilelistPath().string());
  }

// register created data objects with the data reader
  data_reader.registerAcceptances(lmd_acceptances);
  data_reader.registerData(lmd_angular_data);
  data_reader.registerData(lmd_vertex_data);
  data_reader.registerData(lmd_hist_data);
  data_reader.registerMapData(lmd_map_data);

// and read data
  data_reader.read();
}

void PndLmdDataFacade::saveDataToFiles() {
  std::cout << "Saving data....\n";

  std::string filename_extension("");

// add input directory to data facade
  if (lmd_runtime_config.getRawDataFilelistPath().compare("") != 0) {
    const boost::regex my_filename_filter("filelist_(\\d*).txt",
        boost::regex::extended | boost::regex::icase);

    boost::smatch dwhat;
    if (boost::regex_search(
        boost::filesystem::path(lmd_runtime_config.getRawDataFilelistPath()).filename().string(),
        dwhat, my_filename_filter)) {
      filename_extension = "_";
      filename_extension += std::string(dwhat[1]);
    }
  }

  std::string output_filename_angular_data(
      lmd_runtime_config.getElasticDataName());
  std::string output_filename_acceptance_data(
      lmd_runtime_config.getAccDataName());
  std::string output_filename_resolution_data(
      lmd_runtime_config.getResDataName());
  std::string output_filename_vertex_data(
      lmd_runtime_config.getVertexDataName());

  if (filename_extension != "") {
    boost::filesystem::path p(output_filename_angular_data);
    output_filename_angular_data = p.stem().string() + filename_extension
        + p.extension().string();
    p = output_filename_acceptance_data;
    output_filename_acceptance_data = p.stem().string() + filename_extension
        + p.extension().string();
    p = output_filename_resolution_data;
    output_filename_resolution_data = p.stem().string() + filename_extension
        + p.extension().string();
    p = output_filename_vertex_data;
    output_filename_vertex_data = p.stem().string() + filename_extension
        + p.extension().string();
  }

  if (lmd_angular_data.size() > 0) {
    // ---- dpm or elastic data ---- //

    // ---- Output file -------------------------------------------------------
    output_filename_angular_data =
        lmd_runtime_config.getDataOutputDirectory().string() + "/"
            + output_filename_angular_data;
    // ------------------------------------------------------------------------
    TFile f(output_filename_angular_data.c_str(), "RECREATE");
    for (unsigned int i = 0; i < lmd_angular_data.size(); i++) {
      lmd_angular_data[i].saveToRootFile();
    }
    f.Close();
  }
  if (lmd_acceptances.size() > 0) {
    // ---- acceptance or box gen data ---- //

    // ---- Output file -------------------------------------------------------
    output_filename_acceptance_data =
        lmd_runtime_config.getDataOutputDirectory().string() + "/"
            + output_filename_acceptance_data;
    // ------------------------------------------------------------------------
    TFile f(output_filename_acceptance_data.c_str(), "RECREATE");
    for (unsigned int i = 0; i < lmd_acceptances.size(); i++) {
      lmd_acceptances[i].saveToRootFile();
    }
    f.Close();
  }
  if (lmd_hist_data.size() > 0 || lmd_map_data.size() > 0) {
    // ---- create lmd resolution objects from box gen data ---- //

    // ---- Output file -------------------------------------------------------
    output_filename_resolution_data =
        lmd_runtime_config.getDataOutputDirectory().string() + "/"
            + output_filename_resolution_data;
    // ------------------------------------------------------------------------
    TFile f(output_filename_resolution_data.c_str(), "RECREATE");
    for (unsigned int i = 0; i < lmd_hist_data.size(); i++) {
      lmd_hist_data[i].saveToRootFile();
    }
    for (unsigned int i = 0; i < lmd_map_data.size(); i++) {
      lmd_map_data[i].saveToRootFile();
    }
    f.Close();
  }
  if (lmd_vertex_data.size() > 0) {
    // ---- Output file -------------------------------------------------------
    output_filename_vertex_data =
        lmd_runtime_config.getDataOutputDirectory().string() + "/"
            + output_filename_vertex_data;
    // ------------------------------------------------------------------------
    TFile f(output_filename_vertex_data.c_str(), "RECREATE");
    for (unsigned int i = 0; i < lmd_vertex_data.size(); i++) {
      lmd_vertex_data[i].saveToRootFile();
    }
    f.Close();
  }
}

void PndLmdDataFacade::cleanup() {
  clearSelectionDimensionMap();

  lmd_angular_data.clear();
  lmd_vertex_data.clear();
  lmd_hist_data.clear();
  lmd_acceptances.clear();
}

std::vector<PndLmdAngularData> PndLmdDataFacade::getElasticData() const {
  std::vector<PndLmdAngularData> data_vec;
  if (lmd_runtime_config.getElasticDataInputDirectory().string() != "") {
    vector<string> filenames = findFiles(
        lmd_runtime_config.getElasticDataInputDirectory(),
        lmd_runtime_config.getElasticDataName());

    for (auto const& filename : filenames) {
      TFile file(filename.c_str(), "READ");
      auto temp_vec = getDataFromFile<PndLmdAngularData>(file);
      data_vec.insert(data_vec.end(), temp_vec.begin(), temp_vec.end());
    }
  }
  return data_vec;
}

std::vector<PndLmdAcceptance> PndLmdDataFacade::getAcceptanceData() const {
  std::vector<PndLmdAcceptance> data_vec;
  if (lmd_runtime_config.getAcceptanceResolutionInputDirectory().string()
      != "") {
    vector<string> filenames = findFiles(
        lmd_runtime_config.getAcceptanceResolutionInputDirectory(),
        lmd_runtime_config.getAccDataName());

    for (auto const& filename : filenames) {
      TFile file(filename.c_str(), "READ");
      auto temp_vec = getDataFromFile<PndLmdAcceptance>(file);
      data_vec.insert(data_vec.end(), temp_vec.begin(), temp_vec.end());
    }
  }
  return data_vec;
}
std::vector<PndLmdHistogramData> PndLmdDataFacade::getResolutionData() const {
  std::vector<PndLmdHistogramData> data_vec;
  if (lmd_runtime_config.getAcceptanceResolutionInputDirectory().string()
      != "") {
    vector<string> filenames = findFiles(
        lmd_runtime_config.getAcceptanceResolutionInputDirectory(),
        lmd_runtime_config.getResDataName());

    for (auto const& filename : filenames) {
      TFile file(filename.c_str(), "READ");
      auto temp_vec = getDataFromFile<PndLmdHistogramData>(file);
      data_vec.insert(data_vec.end(), temp_vec.begin(), temp_vec.end());
    }
  }
  return data_vec;
}
std::vector<PndLmdHistogramData> PndLmdDataFacade::getVertexData() const {
  std::vector<PndLmdHistogramData> data_vec;
  if (lmd_runtime_config.getElasticDataInputDirectory().string() != "") {
    vector<string> filenames = findFiles(
        lmd_runtime_config.getElasticDataInputDirectory(),
        lmd_runtime_config.getVertexDataName());

    for (auto const& filename : filenames) {
      TFile file(filename.c_str(), "READ");
      auto temp_vec = getDataFromFile<PndLmdHistogramData>(file);
      data_vec.insert(data_vec.end(), temp_vec.begin(), temp_vec.end());
    }
  }
  return data_vec;
}

std::vector<PndLmdMapData> PndLmdDataFacade::getMapData() const {
  std::vector<PndLmdMapData> data_vec;
  if (lmd_runtime_config.getAcceptanceResolutionInputDirectory().string()
      != "") {
    vector<string> filenames = findFiles(
        lmd_runtime_config.getAcceptanceResolutionInputDirectory(),
        lmd_runtime_config.getResDataName());

    for (auto const& filename : filenames) {
      TFile file(filename.c_str(), "READ");
      auto temp_vec = getDataFromFile<PndLmdMapData>(file);
      data_vec.insert(data_vec.end(), temp_vec.begin(), temp_vec.end());
    }
  }
  return data_vec;
}

vector<string> PndLmdDataFacade::findFiles(const path & dir_path,
    const string & file_name) const {
  std::cout << "finding files with pattern " << file_name << " in "
      << dir_path.string() << std::endl;
  vector<string> all_matching_files;

  const boost::regex my_filename_filter(file_name,
      boost::regex::extended | boost::regex::icase);

  directory_iterator end_itr;

  for (directory_iterator fitr(dir_path); fitr != end_itr; ++fitr) {

    boost::smatch fwhat;

    // Skip if no match
    if (!boost::regex_search(fitr->path().filename().string(), fwhat,
        my_filename_filter))
      continue;

    //std::cout << "adding " << fitr->path().string() << " to filelist"
    //    << std::endl;
    all_matching_files.push_back(fitr->path().string());
  }
  return all_matching_files;
}

vector<string> PndLmdDataFacade::findFile(const path & dir_path,
    const string &dir_pattern, const string & file_name) const {

  const boost::regex my_dir_filter(dir_pattern,
      boost::regex::extended | boost::regex::icase);

  vector<string> all_matching_files;

  if (exists(dir_path)) {
    //std::cout << dir_path.string() << " exists... Looping over this directory.."
    //    << std::endl;
    directory_iterator end_itr; // default construction yields past-the-end
    for (directory_iterator itr(dir_path); itr != end_itr; ++itr) {
      // Skip if a file
      if (boost::filesystem::is_regular_file(itr->status()))
        continue;

      if (boost::filesystem::is_directory(itr->status())) {

        // check if directory name matches the dir pattern
        boost::smatch dwhat;
        if (!boost::regex_search(itr->path().filename().string(), dwhat,
            my_dir_filter))
          continue;

        vector<string> found_files = findFiles(itr->path(), file_name);
        all_matching_files.insert(all_matching_files.end(), found_files.begin(),
            found_files.end());
      }
    }
  }
  return all_matching_files;
}

std::vector<PndLmdAcceptance> PndLmdDataFacade::getMatchingAcceptances(
    const std::vector<PndLmdAcceptance> &acceptance_pool,
    const PndLmdAngularData &lmd_data) const {
  std::vector<PndLmdAcceptance> matching_acceptances;

  // --- filter resolutions for the correct dimension options
  PndLmdDataFacade lmd_data_facade;

  // filter the vector for specific options
  LumiFit::LmdDimensionOptions lmd_dim_opt(
      lmd_data.getPrimaryDimension().dimension_options);
  lmd_dim_opt.track_type = LumiFit::MC;

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter prim_dim_filter(
      lmd_dim_opt);

  std::vector<PndLmdAcceptance> prefiltered_acceptances =
      lmd_data_facade.filterData<PndLmdAcceptance>(acceptance_pool,
          prim_dim_filter);

  // find all acceptances with the same selections like the data
  // but ignore secondary track filter as well as in acceptance as in the data selector set
  LumiFit::Comparisons::SelectionDimensionsFilterIgnoringSecondaryTrack selection_dim_filter(
      lmd_data.getSelectorSet());

  matching_acceptances = lmd_data_facade.filterData<PndLmdAcceptance>(
      prefiltered_acceptances, selection_dim_filter);

  return matching_acceptances;
}
std::vector<PndLmdHistogramData> PndLmdDataFacade::getMatchingResolutions(
    const std::set<PndLmdHistogramData> &resolution_pool,
    const PndLmdAngularData &lmd_data) const {
  std::vector<PndLmdHistogramData> matching_resolutions;

  // --- filter resolutions for the correct dimension options
  PndLmdDataFacade lmd_data_facade;

  // filter the vector for specific options
  LumiFit::LmdDimensionOptions lmd_dim_opt(
      lmd_data.getPrimaryDimension().dimension_options);
  lmd_dim_opt.track_type = LumiFit::DIFF_RECO_MC;

  LumiFit::Comparisons::DataPrimaryDimensionOptionsFilter prim_dim_filter(
      lmd_dim_opt);

  std::vector<PndLmdHistogramData> prefiltered_resolutions =
      lmd_data_facade.filterData<PndLmdHistogramData>(resolution_pool,
          prim_dim_filter);

  // TODO: find all resolutions with the same selections like the data
  // but ignore secondary track filter
  // as well as the mc selection dimension in the same variable as the dimensions
  /*LumiFit::Comparisons::SelectionDimensionsFilterIgnoringSecondaryTrack selection_dim_filter(
   lmd_data.getSelectorSet());

   matching_resolutions = lmd_data_facade.filterData<PndLmdHistogramData>(
   prefiltered_resolutions, selection_dim_filter);*/
  matching_resolutions = prefiltered_resolutions;

  return matching_resolutions;
}

std::vector<std::string> PndLmdDataFacade::findFilesByName(
    const boost::filesystem::path &top_dir_path_to_search,
    const std::string dir_name_filter, const std::string file_name) {

  const boost::regex my_filter(dir_name_filter,
      boost::regex::extended | boost::regex::icase);
  const boost::regex my_filename_filter(file_name,
      boost::regex::extended | boost::regex::icase);

  std::vector<std::string> all_matching_files;

  if (exists(top_dir_path_to_search)) {
    // try to find directory pattern
    bool dirname_ok(false);
    boost::smatch what;
    std::cout << "trying find " << dir_name_filter << " within "
        << top_dir_path_to_search.string() << std::endl;
    if (boost::regex_search(top_dir_path_to_search.string(), what, my_filter)) {
      dirname_ok = true;
      std::cout << "This directory matches the filter " << dir_name_filter
          << std::endl;
    }

    std::cout << top_dir_path_to_search.string()
        << " exists... Looping over this directory.." << std::endl;
    boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
    for (boost::filesystem::directory_iterator itr(top_dir_path_to_search);
        itr != end_itr; ++itr) {
      // if the current entry is a file, just test for the filename
      if (boost::filesystem::is_regular_file(itr->status())) {
        if (dirname_ok) {
          // File matches, check if fit_result.root file resides in this directory
          boost::smatch fwhat;

          // Skip if no match
          if (!boost::regex_search(itr->path().string(), fwhat,
              my_filename_filter))
            continue;

          all_matching_files.push_back(itr->path().string());
        }
      }

      else if (boost::filesystem::is_directory(itr->status())) {
        std::vector<std::string> found_files = findFilesByName(itr->path(),
            dir_name_filter, file_name);
        all_matching_files.insert(all_matching_files.end(), found_files.begin(),
            found_files.end());
      }
    }
  }
  std::cout << "Found a total of " << all_matching_files.size()
      << " matching files!" << std::endl;
  return all_matching_files;
}

template<> std::vector<PndLmdMapData> PndLmdDataFacade::getDataFromFile(
    TFile& f) const {
  std::vector<PndLmdMapData> lmd_data_vec;

  unsigned int counter = 0;

  TIter next(f.GetListOfKeys());
  TKey *key;
  while ((key = (TKey*) next())) {

    PndLmdMapData* data;
    f.GetObject(key->GetName(), data);
    if (data) {
      counter++;
      data->convertFromRootTree();
      lmd_data_vec.push_back(*data);
      delete data; // this delete is crucial! otherwise we have a memory leak!
    }
  }
  //std::cout << "Found " << counter << " objects!" << std::endl;
  return lmd_data_vec;
}

/*std::map<LumiFit::LmdSimIPParameters,
 std::map<LumiFit::LmdSimIPParameters,
 std::map<LumiFit::LmdDimensionType, std::vector<PndLmdHistogramData> > > > PndLmdDataFacade::clusterVertexData(
 std::vector<PndLmdHistogramData> &lmd_vertex_vec) {
 std::map<LumiFit::LmdSimIPParameters,
 std::map<LumiFit::LmdSimIPParameters,
 std::map<LumiFit::LmdDimensionType, std::vector<PndLmdHistogramData> > > > return_map;

 for (unsigned int i = 0; i < lmd_vertex_vec.size(); i++) {
 LumiFit::LmdSimIPParameters sim_ip_params =
 lmd_vertex_vec[i].getSimulationIPParameters();

 sim_ip_params.offset_x_mean = 0.0; // set the insensitive value to 0.0
 LumiFit::LmdSimIPParameters dependency_ip_params;
 dependency_ip_params.offset_x_mean = 1.0; // set the dependency value to 1.0
 return_map[dependency_ip_params][sim_ip_params][lmd_vertex_vec[i].getPrimaryDimension().dimension_options.dimension_type].push_back(
 lmd_vertex_vec[i]);

 sim_ip_params = lmd_vertex_vec[i].getSimulationIPParameters();
 sim_ip_params.offset_y_mean = 0.0; // set the insensitive value to 0.0
 dependency_ip_params.reset();
 dependency_ip_params.offset_y_mean = 1.0; // set the dependency value to 1.0
 return_map[dependency_ip_params][sim_ip_params][lmd_vertex_vec[i].getPrimaryDimension().dimension_options.dimension_type].push_back(
 lmd_vertex_vec[i]);

 sim_ip_params = lmd_vertex_vec[i].getSimulationIPParameters();
 sim_ip_params.offset_z_mean = 0.0; // set the insensitive value to 0.0
 dependency_ip_params.reset();
 dependency_ip_params.offset_z_mean = 1.0; // set the dependency value to 1.0
 return_map[dependency_ip_params][sim_ip_params][lmd_vertex_vec[i].getPrimaryDimension().dimension_options.dimension_type].push_back(
 lmd_vertex_vec[i]);

 sim_ip_params = lmd_vertex_vec[i].getSimulationIPParameters();
 sim_ip_params.offset_x_width = 0.0; // set the insensitive value to 0.0
 dependency_ip_params.reset();
 dependency_ip_params.offset_x_width = 1.0; // set the dependency value to 1.0
 return_map[dependency_ip_params][sim_ip_params][lmd_vertex_vec[i].getPrimaryDimension().dimension_options.dimension_type].push_back(
 lmd_vertex_vec[i]);

 sim_ip_params = lmd_vertex_vec[i].getSimulationIPParameters();
 sim_ip_params.offset_y_width = 0.0; // set the insensitive value to 0.0
 dependency_ip_params.reset();
 dependency_ip_params.offset_y_width = 1.0; // set the dependency value to 1.0
 return_map[dependency_ip_params][sim_ip_params][lmd_vertex_vec[i].getPrimaryDimension().dimension_options.dimension_type].push_back(
 lmd_vertex_vec[i]);

 sim_ip_params = lmd_vertex_vec[i].getSimulationIPParameters();
 sim_ip_params.offset_z_width = 0.0; // set the insensitive value to 0.0
 dependency_ip_params.reset();
 dependency_ip_params.offset_z_width = 1.0; // set the dependency value to 1.0
 return_map[dependency_ip_params][sim_ip_params][lmd_vertex_vec[i].getPrimaryDimension().dimension_options.dimension_type].push_back(
 lmd_vertex_vec[i]);
 }

 return return_map;
 }*/
