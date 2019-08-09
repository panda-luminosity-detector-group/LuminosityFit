#include "fit/data/DataStructs.h"
#include "ui/PndLmdDataFacade.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"
#include "data/PndLmdMapData.h"

#include "TFile.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <random>

#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations
#include "boost/regex.hpp"

using std::set;
using std::vector;
using std::string;
using boost::filesystem::path;
using boost::filesystem::directory_iterator;

std::string getOutputFilename(const std::string& data_type) {
  std::string output_filename("");
  if (data_type.find("a") != std::string::npos)
    output_filename = "lmd_data";
  else if (data_type.find("e") != std::string::npos)
    output_filename = "lmd_acc_data";
  else if (data_type.find("r") != std::string::npos)
    output_filename = "lmd_res_data";
  else if (data_type.find("h") != std::string::npos)
    output_filename = "lmd_res_hist_data";
  else if (data_type.find("v") != std::string::npos)
    output_filename = "lmd_vertex_data";
  return output_filename;
}

unsigned int binominalCoefficient(unsigned int n, unsigned int k) {
  unsigned int combinations(1);

  // assumes n > k
  if (k > n - k) {
    for (unsigned int i = n; i > k; --i)
      combinations *= i;
    unsigned int denom(1);
    for (unsigned int i = n - k; i > 1; --i)
      denom *= i;

    combinations = combinations / denom;
  } else {
    for (unsigned int i = n; i > n - k; --i)
      combinations *= i;
    unsigned int denom(1);
    for (unsigned int i = k; i > 1; --i)
      denom *= i;

    combinations = combinations / denom;
  }
  return combinations;
}

vector<vector<string> > bootstrapData(vector<string> found_files,
    std::pair<unsigned int, unsigned int> samples) {

  std::cout << "bootstraping data...\n";
  std::cout << "creating " << samples.first << " samples with "
      << samples.second << " out of " << found_files.size() << " files\n";

  set<vector<unsigned int> > sample_lists;

  // obtain a seed from the system clock:
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::mt19937 generator(seed); // mt19937 is a standard mersenne_twister_engine
  std::uniform_int_distribution<unsigned int> distribution(0,
      found_files.size() - 1);

  unsigned int max_trys(10000);
  unsigned int trys(0);

  while (sample_lists.size() <= samples.first && trys < max_trys) {
    ++trys;
    vector<unsigned int> temp_vec;
    for (unsigned int i = 0; i < samples.second; ++i) {
      unsigned int number = distribution(generator);
      temp_vec.push_back(number);
    }
    std::sort(temp_vec.begin(), temp_vec.end());
    sample_lists.insert(temp_vec);
  }

  vector<vector<string> > file_lists;
  for (auto const& sample_list : sample_lists) {
    vector<string> temp_vec;
    for (auto index : sample_list) {
      temp_vec.push_back(found_files[index]);
    }
    file_lists.push_back(temp_vec);
  }
  return file_lists;
}

template<class T> void mergeData(const vector<string>& found_files,
    const string& outfile_path, const string& data_type,
    std::pair<unsigned int, unsigned int> samples) {
  string output_filename = getOutputFilename(data_type);

  vector<vector<string> > data_file_samples = bootstrapData(found_files,
      samples);

  boost::filesystem::path outdir(outfile_path);
  boost::filesystem::create_directory(outdir);

  std::cout << "Merging data for " << data_file_samples.size() << " samples!\n";
  std::cout << "Each sample contains " << data_file_samples[0].size() << " files\n";
  for (unsigned int i = 0; i < data_file_samples.size(); ++i) {
    std::stringstream ss;
    ss << outfile_path << "/" << output_filename << "_" << i << "of"
        << data_file_samples.size() << ".root";
    // output file
    TFile fmergeddata(ss.str().c_str(), "RECREATE");

    mergeData<T>(data_file_samples[i], &fmergeddata);
  }

  std::cout << "Finished merging!" << std::endl;
}

template<class T> void mergeData(vector<string> found_files,
    TFile *output_file) {
  std::cout << "Attempting to merge files...\n";
// A small helper class that helps to construct lmddata objects
  PndLmdDataFacade lmd_data_facade;

  set<T> merged_files;
  typename set<T>::iterator iter;
  typename vector<T>::iterator entry;
  std::pair<typename set<T>::iterator, bool> ret;

  for (unsigned int i = 0; i < found_files.size(); i++) {
    TFile fdata(found_files[i].c_str(), "READ");

    // get lmd data and objects from files
    vector<T> lmd_data_vec = lmd_data_facade.getDataFromFile<T>(fdata);

    output_file->cd();
    for (entry = lmd_data_vec.begin(); entry != lmd_data_vec.end(); entry++) {
      iter = merged_files.find(*entry);

      if (iter == merged_files.end()) {
        ret = merged_files.insert(*entry);
        //((PndLmdAbstractData*) &(*ret.first))->cloneData(*lmd_data);
      } else {
        PndLmdAbstractData *lmd_data_merge = (PndLmdAbstractData*) &(*iter);
        lmd_data_merge->add(*((PndLmdAbstractData*) &(*entry)));
      }
    }
  }

  output_file->cd();

  for (iter = merged_files.begin(); iter != merged_files.end(); iter++) {
    ((PndLmdAbstractData*) &(*iter))->saveToRootFile();
  }
}

void displayInfo() {
// display info
  std::cout << "Required arguments are: " << std::endl;
  std::cout << "-p [path to data]" << std::endl;
  std::cout
      << "-t [type of data to create] (a = angular, e = efficiency, r = resolution, v = vertex)"
      << std::endl;
  std::cout << "Optional arguments are: " << std::endl;
  std::cout << "-n [number of samples]" << std::endl;
  std::cout << "-s [samples size]" << std::endl;
  std::cout << "-f [filename pattern] (default: lmd_data.root etc.)"
      << std::endl;
  std::cout << "-d [directory pattern] (default: bunch)" << std::endl;
  std::cout << std::endl;
  std::cout
      << "Note: The type value is specified as a string, in which the 4 letters\n"
          "a, e, r, v can be used. Only use a SINGLE data type at once. "
          "Do not combine!\n" << std::endl;
}

bool checkDataType(std::string& data_type) {
  bool is_valid(false);
  if (data_type.size() == 1) {
    if (data_type.find("a") != std::string::npos) {
      is_valid = true;
    } else if (data_type.find("e") != std::string::npos) {
      is_valid = true;
    } else if (data_type.find("r") != std::string::npos) {
      is_valid = true;
    } else if (data_type.find("h") != std::string::npos) {
      is_valid = true;
    } else if (data_type.find("v") != std::string::npos) {
      is_valid = true;
    }
  }
  return is_valid;
}

int main(int argc, char* argv[]) {
  bool is_data_path_set = false;
  bool is_filename_pattern_set = false;
  bool is_type_set = false;
  string filename_pattern("lmd_data.root");
  string dir_pattern("");
  string data_path;
  std::string data_type("");
  unsigned int num_samples(1);
  unsigned int sample_size(0);
  int c;

  while ((c = getopt(argc, argv, "hf:p:d:t:s:n:")) != -1) {
    switch (c) {
    case 'f':
      filename_pattern = optarg;
      is_filename_pattern_set = true;
      break;
    case 'p':
      data_path = optarg;
      is_data_path_set = true;
      break;
    case 'd':
      dir_pattern = optarg;
      break;
    case 'n':
      num_samples = std::stoi(optarg);
      break;
    case 's':
      sample_size = std::stoi(optarg);
      break;
    case 't':
      data_type = optarg;
      is_type_set = true;
      break;
    case '?':
      if (optopt == 'f' || optopt == 'p' || optopt == 'd' || optopt == 's'
          || optopt == 't')
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

  if (is_data_path_set && is_type_set) {
    if (!is_filename_pattern_set) {
      if (data_type.find("e") != std::string::npos)
        filename_pattern = "lmd_acc_data.root";
      else if (data_type.find("r") != std::string::npos
          || data_type.find("h") != std::string::npos)
        filename_pattern = "lmd_res_data.root";
      else if (data_type.find("v") != std::string::npos)
        filename_pattern = "lmd_vertex_data.root";
    }

    PndLmdDataFacade lmd_data_facade;

    vector<string> found_files;
    if (dir_pattern.compare("") != 0) {
      // ------ get files ------
      found_files = lmd_data_facade.findFile(data_path, dir_pattern,
          filename_pattern);
    } else {
      found_files = lmd_data_facade.findFiles(data_path, filename_pattern);
    }

    std::pair<unsigned int, unsigned int> samples;
    if (num_samples == 0)
      num_samples = 1; // default is one sample
    samples.first = num_samples;
    samples.second = sample_size;
    // automatic setting of sample size
    if (sample_size == 0) {
      if (num_samples == 1) {
        // create one sample with with full size
        samples.second = found_files.size();
      }
      else {
        // user wants more than one sample. Use bootstrapping with a samples
        // size of half the found files for each bootstrapped sample
        samples.second =  found_files.size()/2;
      }
    }
    size_t max_bootstrap_samples = binominalCoefficient(found_files.size(), samples.second);
    if(max_bootstrap_samples < num_samples) {
      std::cout << "WARNING: requested number of bootstraped samples is higher"
                << " than the maximum possible from combinatorics!"
                << " Using maximum of " << max_bootstrap_samples << " instead!"
                << std::endl;
    }

    std::string outpath = data_path + "/merge_data";

    if (data_type.find("a") != std::string::npos) {
      mergeData<PndLmdAngularData>(found_files, outpath, data_type, samples);
    } else if (data_type.find("e") != std::string::npos) {
      mergeData<PndLmdAcceptance>(found_files, outpath, data_type, samples);
    } else if (data_type.find("r") != std::string::npos) {
      mergeData<PndLmdMapData>(found_files, outpath, data_type, samples);
    } else if (data_type.find("h") != std::string::npos) {
      mergeData<PndLmdHistogramData>(found_files, outpath, data_type, samples);
    } else if (data_type.find("v") != std::string::npos) {
      mergeData<PndLmdHistogramData>(found_files, outpath, data_type, samples);
    }
  } else
    displayInfo();

  return 0;
}
