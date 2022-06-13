/*
 * calculateElasticCrossSection.cxx
 *
 *  Created on: Apr 7, 2014
 *      Author: steve
 */

#include "TFile.h"

#include "boost/filesystem.hpp"

#include <fstream>

#include "tools/PbarPElasticScatteringEventGenerator.h"

void displayInfo() {
  // display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "[pbar momentum]" << std::endl;
  std::cout << "[number of events]" << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-l [lower theta boundary in mrad] (default: 2mrad)"
            << std::endl;
  std::cout << "-u [upper theta boundary in mrad] (default: 12mrad)"
            << std::endl;
  std::cout << "-o [output filepath] (default: dpmgen.root)" << std::endl;
  std::cout << "-n [lower phi boundary in rad] (default: 0)"
            << std::endl;
  std::cout << "-g [upper phi boundary in rad] (default: 2 * TMath::Pi())"
            << std::endl;
  std::cout << "-s [random seed] (default: 12345)" << std::endl;
  
}

int main(int argc, char *argv[]) {
  double lower_bound = 2.0;
  double upper_bound = 12.0;
  std::string output_filepath = "dpmgen.root";
  unsigned int seed = 12345;
  double lower_phi = 0;
  double upper_phi = 2 * TMath::Pi();
  int c;

  while ((c = getopt(argc, argv, "hl:u:o:n:g:s:")) != -1) {
    switch (c) {
    case 'o':
      output_filepath = optarg;
      break;
    case 's':
      seed = std::stoul(optarg);
      break;
    case 'l':
      lower_bound = std::stod(optarg);
      break;
    case 'u':
      upper_bound = std::stod(optarg);
      break;
    case 'n':
      lower_phi = std::stod(optarg);
      break;
    case 'g':
      upper_phi = std::stod(optarg);
      break;
    case '?':
      if (optopt == 'l' || optopt == 'u' || optopt == 'o' || optopt == 'n' || optopt == 'g'|| optopt == 's' )
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

  if (argc - optind == 2) {
    double momentum = std::stod(argv[optind]);
    unsigned int num_events = std::stoul(argv[optind + 1]);
    auto result = PbarPElasticScattering::generateEvents(
        momentum, num_events, lower_bound, upper_bound, lower_phi, upper_phi, seed);

    if (num_events == 0) {
      boost::filesystem::path cs_filepath(output_filepath);
      cs_filepath =
          cs_filepath.parent_path().string() + "/elastic_cross_section.txt";

      std::ofstream myfile;
      myfile.open(cs_filepath.string());
      myfile << result.second;
      myfile.close();
      return 0;
    }

    TFile f(output_filepath.c_str(), "RECREATE");
    auto tree = result.first;

    std::cout << "writing to file " << output_filepath << " ...\n";
    if (num_events > 0) {
      if (tree == nullptr) {
        std::cout << "ERROR: tree should be a nullptr!\n";
      }
      tree->Write();
    }
    f.Close();
  } else
    displayInfo();

  return 0;
}
