#include "PndLmdRuntimeConfiguration.h"

#include "data/PndLmdDataFacade.h"
#include "fit/PndLmdFitFacade.h"

#include <iostream>

#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ini_parser.hpp"

using boost::filesystem::path;
using boost::property_tree::ptree;

PndLmdRuntimeConfiguration::PndLmdRuntimeConfiguration() :
		number_of_threads(1), elastic_data_name("lmd_data.root"), acc_data_name(
				"lmd_acc_data.root"), res_data_name("lmd_res_data.root"), res_param_data_name(
				"resolution_params_1.root"), fitted_elastic_data_name(
				"lmd_fitted_data.root"), vertex_data_name("lmd_vertex_data.root") {
	//selection_dimensions.first = "";
	//selection_dimensions.second = std::vector<LumiFit::LmdDimension>();
}

PndLmdRuntimeConfiguration::~PndLmdRuntimeConfiguration() {
}

void PndLmdRuntimeConfiguration::readSimulationParameters(
		const std::string& file_url) {
// read the config file
	std::cout << "Trying to read config file: " << file_url << std::endl;
	read_ini(file_url, simulation_parameter_tree);
}

void PndLmdRuntimeConfiguration::readFitConfigFile(
		const std::string &file_url) {
	// read the config file
	read_json(file_url, fit_config_tree);
}

void PndLmdRuntimeConfiguration::readDataConfigFile(
		const std::string &file_url) {
	// read the config file
	read_json(file_url, data_config_tree);
}

const ptree& PndLmdRuntimeConfiguration::getSimulationParameters() const {
	return simulation_parameter_tree;
}

unsigned int PndLmdRuntimeConfiguration::getNumberOfThreads() const {
	return number_of_threads;
}
double PndLmdRuntimeConfiguration::getMomentum() const {
	return momentum;
}
unsigned int PndLmdRuntimeConfiguration::getNumEvents() const {
	return num_events;
}
double PndLmdRuntimeConfiguration::getTotalElasticCrossSection() const {
	return total_elastic_cross_section;
}

const std::string& PndLmdRuntimeConfiguration::getElasticDataName() const {
	return elastic_data_name;
}
const std::string& PndLmdRuntimeConfiguration::getAccDataName() const {
	return acc_data_name;
}
const std::string& PndLmdRuntimeConfiguration::getFittedElasticDataName() const {
	return fitted_elastic_data_name;
}
const std::string& PndLmdRuntimeConfiguration::getResDataName() const {
	return res_data_name;
}
const std::string& PndLmdRuntimeConfiguration::getResParamDataName() const {
	return res_param_data_name;
}
const std::string& PndLmdRuntimeConfiguration::getVertexDataName() const {
	return vertex_data_name;
}

const path& PndLmdRuntimeConfiguration::getElasticDataInputDirectory() const {
	return elastic_data_input_directory;
}
const path& PndLmdRuntimeConfiguration::getAcceptanceResolutionInputDirectory() const {
	return acceptance_resolution_input_directory;
}
const path& PndLmdRuntimeConfiguration::getReferenceAcceptanceResolutionInputDirectory() const {
	return reference_acceptance_resolution_input_directory;
}

const path& PndLmdRuntimeConfiguration::getRawDataDirectory() const {
	return raw_data_directory;
}
const path& PndLmdRuntimeConfiguration::getRawDataFilelistPath() const {
	return raw_data_filelist_path;
}
const path& PndLmdRuntimeConfiguration::getDataOutputDirectory() const {
	return data_output_directory;
}

bool PndLmdRuntimeConfiguration::isAutomaticPrimaryResolutionRangeActive() const {
	return automatic_primary_resolution_range_active;
}
bool PndLmdRuntimeConfiguration::isAutomaticSecondaryResolutionRangeActive() const {
	return automatic_secondary_resolution_range_active;
}

const ptree& PndLmdRuntimeConfiguration::getDataConfigTree() const {
	return data_config_tree;
}
const ptree& PndLmdRuntimeConfiguration::getFitConfigTree() const {
	return fit_config_tree;
}

void PndLmdRuntimeConfiguration::setNumberOfThreads(
		unsigned int number_of_threads_) {
	number_of_threads = number_of_threads_;
}
void PndLmdRuntimeConfiguration::setMomentum(double momentum_) {
	momentum = momentum_;
}
void PndLmdRuntimeConfiguration::setNumEvents(unsigned int num_events_) {
	num_events = num_events_;
}
void PndLmdRuntimeConfiguration::setTotalElasticCrossSection(
		double total_elastic_cross_section_) {
	total_elastic_cross_section = total_elastic_cross_section_;
}

void PndLmdRuntimeConfiguration::setElasticDataName(
		const std::string& elastic_data_name_) {
	elastic_data_name = elastic_data_name_;
}
void PndLmdRuntimeConfiguration::setAccDataName(
		const std::string& acc_data_name_) {
	acc_data_name = acc_data_name_;
}
void PndLmdRuntimeConfiguration::setFittedElasticDataName(
		const std::string& fitted_elastic_data_name_) {
	fitted_elastic_data_name = fitted_elastic_data_name_;
}
void PndLmdRuntimeConfiguration::setResDataName(
		const std::string& res_data_name_) {
	res_data_name = res_data_name_;
}
void PndLmdRuntimeConfiguration::setResParamDataName(
		const std::string& res_param_data_name_) {
	res_param_data_name = res_param_data_name_;
}
void PndLmdRuntimeConfiguration::setVertexDataName(
		const std::string& vertex_data_name_) {
	vertex_data_name = vertex_data_name_;
}

void PndLmdRuntimeConfiguration::setElasticDataInputDirectory(
		const std::string& elastic_data_input_directory_) {
	path pathname(elastic_data_input_directory_);
	if (boost::filesystem::exists(pathname)
			&& boost::filesystem::is_directory(pathname)) {
		elastic_data_input_directory = pathname;
	} else {
		std::runtime_error(
				"PndLmdRuntimeConfiguration: elastic data directory containing a PndLmdAngularData data does not exist! Please set correctly!");
	}
}
void PndLmdRuntimeConfiguration::setAcceptanceResolutionInputDirectory(
		const std::string& acceptance_resolution_input_directory_) {
	path pathname(acceptance_resolution_input_directory_);
	if (boost::filesystem::exists(pathname)
			&& boost::filesystem::is_directory(pathname)) {
		acceptance_resolution_input_directory = pathname;
	} else {
		std::runtime_error(
				"PndLmdRuntimeConfiguration: acceptance data directory containing a PndLmdAcceptance data does not exist! Please set correctly!");
	}
}
void PndLmdRuntimeConfiguration::setReferenceAcceptanceResolutionInputDirectory(
		const std::string& reference_acceptance_resolution_input_directory_) {
	path pathname(reference_acceptance_resolution_input_directory_);
	if (boost::filesystem::exists(pathname)
			&& boost::filesystem::is_directory(pathname)) {
		reference_acceptance_resolution_input_directory = pathname;
	} else {
		std::runtime_error(
				"PndLmdRuntimeConfiguration: reference acceptance data directory containing a PndLmdAcceptance data does not exist! Please set correctly!");
	}
}

void PndLmdRuntimeConfiguration::setRawDataDirectory(
		const std::string& raw_data_directory_) {
	path pathname(raw_data_directory_);
	if (boost::filesystem::exists(pathname)
			&& boost::filesystem::is_directory(pathname)) {
		raw_data_directory = pathname;
	} else {
		std::runtime_error(
				"PndLmdRuntimeConfiguration: raw data directory containing pnd lmd track info does not exist! Please set correctly!");
	}
}
void PndLmdRuntimeConfiguration::setRawDataFilelistPath(
		const std::string& raw_data_filelist_path_) {
	path pathname(raw_data_filelist_path_);
	if (boost::filesystem::exists(pathname)
			&& boost::filesystem::is_regular_file(pathname)) {
		raw_data_filelist_path = pathname;
	} else {
		std::runtime_error(
				"PndLmdRuntimeConfiguration: raw data filelist directory containing pnd lmd track info URLs does not exist! Please set correctly!");
	}
}
void PndLmdRuntimeConfiguration::setDataOutputDirectory(
		const std::string& data_output_directory_) {
	path pathname(data_output_directory_);
	if (boost::filesystem::exists(pathname)
			&& boost::filesystem::is_directory(pathname)) {
		data_output_directory = pathname;
	} else {
		std::runtime_error(
				"PndLmdRuntimeConfiguration: lmd data output directory containing PndLmdData objects does not exist! Please set correctly!");
	}
}

void PndLmdRuntimeConfiguration::setAutomaticPrimaryResolutionRangeActive(
		bool automatic_primary_resolution_range_active_) {
	automatic_primary_resolution_range_active =
			automatic_primary_resolution_range_active_;
}
void PndLmdRuntimeConfiguration::setAutomaticSecondaryResolutionRangeActive(
		bool automatic_secondary_resolution_range_active_) {
	automatic_secondary_resolution_range_active =
			automatic_secondary_resolution_range_active_;
}

