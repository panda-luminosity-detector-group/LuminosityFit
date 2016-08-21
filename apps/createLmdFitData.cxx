/*
 * This is the application that generates the Lmd data objects, which can be
 * used to determine the luminosity. This application primarily uses the
 * #PndLmdDataFacade class to easily create lmd fit data objects and fill them
 * with data. General information about the individual classes of the LmdFit
 * framework can be found in the doxygen manual.
 * Run it with argument -h to get running help:
 * 
 * ./createLumiFitData -h
 */

#include "ui/PndLmdRuntimeConfiguration.h"
#include "ui/PndLmdDataFacade.h"

#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void createLmdFitData(const std::string &input_dir_path,
    const std::string &filelist_path, const std::string &output_dir_path,
    const std::string &config_file_url, const double mom,
    std::string& data_types, int num_events,
    const double total_elastic_cross_section) {
  std::cout << "Running LmdFit data reader....\n";

  PndLmdRuntimeConfiguration& lmd_runtime_config =
      PndLmdRuntimeConfiguration::Instance();

  // set paths and general infos first
  lmd_runtime_config.setMomentum(mom);
  lmd_runtime_config.setNumEvents(num_events);
  lmd_runtime_config.setTotalElasticCrossSection(total_elastic_cross_section);

  lmd_runtime_config.setRawDataDirectory(input_dir_path);
  lmd_runtime_config.setRawDataFilelistPath(filelist_path);
  lmd_runtime_config.setDataOutputDirectory(output_dir_path);

  // read simulation parameter config
  boost::filesystem::path sim_config_path(
      lmd_runtime_config.getRawDataDirectory().parent_path().string()
          + "/sim_params.config");
  // if one directory up does not contain it just go up one more
  if (!boost::filesystem::is_regular_file(sim_config_path))
    sim_config_path =
        boost::filesystem::path(
            lmd_runtime_config.getRawDataDirectory().parent_path().parent_path().string()
                + "/sim_params.config");
  lmd_runtime_config.readSimulationParameters(sim_config_path.string());
  // read data parameter config
  lmd_runtime_config.readDataConfigFile(config_file_url);

  PndLmdDataFacade data_facade;
  data_facade.createAndFillDataBundles(data_types);

  std::cout << std::endl << std::endl;
  std::cout << "Application finished successfully." << std::endl;
  std::cout << std::endl;

  // ------------------------------------------------------------------------
}

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "-m [pbar momentum]" << std::endl;
  std::cout
      << "-t [type of data to create] (a = angular, e = efficiency, r = resolution, v = vertex)"
      << std::endl;
  std::cout << "-d [input directory path]" << std::endl;
  std::cout << "-c [data config file path]" << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-f [filelist path]" << std::endl;
  std::cout << "-o [output directory path]" << std::endl;
  std::cout << "-n [number of events to process] "
      "(default 0: all data found will be processed)" << std::endl;
  std::cout << "-e [total elastic cross section]" << std::endl;
  std::cout << std::endl;
  std::cout
      << "Note: The type value is specified as a string, in which the 4 letters\n"
          "a, e, r, v can be concatenated freely. For just a single data type\n"
          "the values should be a, e, r, v. For a combination i.e\n"
          "angular vertex data, one should use av or va. Other additional\n"
          "characters will be ignored, so can be specified without any effect!\n"
          "\n"
          "the parameter -e is the total elastic cross section to estimate\n"
          "the generated luminosity. In case you do NOT specify this value it\n"
          "will be set to -1.0 and there is no performance validation possible,\n"
          "but only luminosity determination. This should be the case only for\n"
          "real data!" << std::endl;
}

bool checkDataType(std::string& data_type) {
  bool is_valid = false;
  if (data_type.find("a") != std::string::npos) {
    is_valid = true;
  } else if (data_type.find("e") != std::string::npos) {
    is_valid = true;
  } else if (data_type.find("r") != std::string::npos) {
    is_valid = true;
  } else if (data_type.find("v") != std::string::npos) {
    is_valid = true;
  }
  return is_valid;
}

int main(int argc, char* argv[]) {
  bool is_mom_set = false, is_cross_section_set = false, is_data_path_set =
      false, is_filelist_path_set = false, is_output_data_path_set = false,
      is_config_file_path_set = false;
  double momentum = -1.0;
  std::string data_type = "";
  unsigned int num_events = 0;
  double cross_section = 1.0;
  std::string data_path;
  std::string config_file_path;
  std::string output_dir_path;
  std::string filelist_path("");
  int c;

  while ((c = getopt(argc, argv, "hm:f:o:n:t:d:c:e:")) != -1) {
    switch (c) {
    case 'm':
      momentum = atof(optarg);
      is_mom_set = true;
      break;
    case 'f':
      filelist_path = optarg;
      is_filelist_path_set = true;
      break;
    case 'o':
      output_dir_path = optarg;
      is_output_data_path_set = true;
      break;
    case 'n':
      num_events = atoi(optarg);
      break;
    case 'e':
      cross_section = atof(optarg);
      is_cross_section_set = true;
      break;
    case 't':
      data_type = optarg;
      break;
    case 'd':
      data_path = optarg;
      is_data_path_set = true;
      break;
    case 'c':
      config_file_path = optarg;
      is_config_file_path_set = true;
      break;
    case '?':
      if (optopt == 't' || optopt == 'd' || optopt == 'm' || optopt == 'n'
          || optopt == 'c' || optopt == 'e')
        std::cerr << "Option -" << optopt << " requires an argument."
            << std::endl;
      else if (isprint(optopt))
        std::cerr << "Unknown option -" << optopt << "." << std::endl;
      else
        std::cerr << "Unknown option character" << optopt << "." << std::endl;
      return 1;
    case 'h':
      displayInfo();
      return 1;
    default:
      return 1;
    }
  }

  bool skip_program = false;
  if (!is_output_data_path_set && is_filelist_path_set) {
    std::cerr
        << "Please specify an output directory via the -o option when using a filelist path as input!"
        << std::endl;
    skip_program = true;
  }
  if (!(checkDataType(data_type) && is_mom_set && is_data_path_set))
    skip_program = true;

  if (skip_program)
    displayInfo();
  else {
    if (!is_output_data_path_set)
      output_dir_path = data_path;
    createLmdFitData(data_path, filelist_path, output_dir_path,
        config_file_path, momentum, data_type, num_events, cross_section);

    return 0;
  }
}
