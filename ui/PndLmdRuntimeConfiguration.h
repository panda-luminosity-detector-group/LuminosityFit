/*
 * PndLmdRuntimeConfiguration.h
 *
 *  Created on: Dec 17, 2014
 *      Author: steve
 */

#ifndef PNDLMDRUNTIMECONFIGURATION_H_
#define PNDLMDRUNTIMECONFIGURATION_H_

#include "LumiFitStructs.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/filesystem.hpp"

class PndLmdRuntimeConfiguration {
	//general config
	unsigned int number_of_threads;
	boost::property_tree::ptree general_config_tree;

	// directory paths
	boost::filesystem::path elastic_data_input_directory;
	boost::filesystem::path acceptance_resolution_input_directory;
	boost::filesystem::path reference_acceptance_resolution_input_directory;
	boost::filesystem::path general_config_directory;

	boost::filesystem::path raw_data_directory;
	boost::filesystem::path raw_data_filelist_path;
	boost::filesystem::path data_output_directory;

	// file names
	std::string elastic_data_name;
	std::string acc_data_name;
	std::string res_data_name;
	std::string res_param_data_name;
	std::string fitted_elastic_data_name;
	std::string vertex_data_name;

	// data stuff
	double momentum;
	unsigned int num_events;
	double total_elastic_cross_section;
	boost::property_tree::ptree data_config_tree;

	// fit options
	boost::property_tree::ptree fit_config_tree;

	// simulation options
	boost::property_tree::ptree simulation_parameter_tree;

  // acceptance offset transformation parameters
  boost::property_tree::ptree acceptance_offsets_transformation_tree;

	PndLmdRuntimeConfiguration();

	virtual ~PndLmdRuntimeConfiguration();

public:
	static PndLmdRuntimeConfiguration& Instance() {
		static PndLmdRuntimeConfiguration runtime_config_instance;
		return runtime_config_instance;
	}

  PndLmdRuntimeConfiguration(PndLmdRuntimeConfiguration const&) = delete;
  void operator=(PndLmdRuntimeConfiguration const&) = delete;

	// getters
	unsigned int getNumberOfThreads() const;
	double getMomentum() const;
	unsigned int getNumEvents() const;
	double getTotalElasticCrossSection() const;

	const std::string& getElasticDataName() const;
	const std::string& getAccDataName() const;
	const std::string& getFittedElasticDataName() const;
	const std::string& getResDataName() const;
	const std::string& getResParamDataName() const;
	const std::string& getVertexDataName() const;

	const boost::filesystem::path& getElasticDataInputDirectory() const;
	const boost::filesystem::path& getAcceptanceResolutionInputDirectory() const;
	const boost::filesystem::path& getReferenceAcceptanceResolutionInputDirectory() const;

	const boost::filesystem::path& getRawDataDirectory() const;
	const boost::filesystem::path& getRawDataFilelistPath() const;
	const boost::filesystem::path& getDataOutputDirectory() const;

	const boost::property_tree::ptree& getDataConfigTree() const;
	const boost::property_tree::ptree& getFitConfigTree() const;
  const boost::property_tree::ptree& getAcceptanceOffsetsTranformationParameters() const;

	// setters
	void setNumberOfThreads(unsigned int number_of_threads_);
	void setMomentum(double momentum_);
	void setNumEvents(unsigned int num_events_);
	void setTotalElasticCrossSection(double total_elastic_cross_section_);

	void setGeneralConfigDirectory(
	    const std::string& general_config_directory_);
	void setElasticDataName(const std::string& elastic_data_name_);
	void setAccDataName(const std::string& acc_data_name_);
	void setFittedElasticDataName(const std::string& fitted_elastic_data_name_);
	void setResDataName(const std::string& res_data_name_);
	void setResParamDataName(const std::string& res_param_data_name_);
	void setVertexDataName(const std::string& vertex_data_name_);

	void readFitConfig(
	    const std::string& fit_config_path);
	void readAcceptanceOffsetTransformationParameters(
	    const std::string& acceptance_offsets_tranformation_file_name);
	void readDataConfig(
	    const std::string& data_config_name);

	void setElasticDataInputDirectory(
			const std::string& elastic_data_input_directory_);
	void setAcceptanceResolutionInputDirectory(
			const std::string& acceptance_resolution_input_directory_);
	void setReferenceAcceptanceResolutionInputDirectory(
			const std::string& reference_acceptance_resolution_input_directory_);

	void setRawDataDirectory(const std::string& raw_data_directory_);
	void setRawDataFilelistPath(const std::string& raw_data_filelist_path_);
	void setDataOutputDirectory(const std::string& data_output_directory_);

	// config file read functions
	void readSimulationParameters(const std::string& file_url);

	const boost::property_tree::ptree& getSimulationParameters() const;
};

#endif /* PNDLMDRUNTIMECONFIGURATION_H_ */
