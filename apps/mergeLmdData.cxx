#include "fit/data/DataStructs.h"
#include "ui/PndLmdDataFacade.h"
#include "data/PndLmdAngularData.h"
#include "data/PndLmdAcceptance.h"

#include "TString.h"
#include "TFile.h"

#include <vector>
#include <iostream>

#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations
#include "boost/regex.hpp"

using std::set;
using std::vector;
using std::string;
using boost::filesystem::path;
using boost::filesystem::directory_iterator;

vector<string> find_files(const path & dir_path, const string & file_name) {

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
		//		<< std::endl;
		all_matching_files.push_back(fitr->path().string());
	}
	return all_matching_files;
}

vector<string> find_file(const path & dir_path, const string &dir_pattern,
		const string & file_name) // search for this name
		{

	const boost::regex my_dir_filter(dir_pattern,
			boost::regex::extended | boost::regex::icase);

	vector<string> all_matching_files;

	if (exists(dir_path)) {
		//std::cout << dir_path.string() << " exists... Looping over this directory.."
		//		<< std::endl;
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

				vector<string> found_files = find_files(itr->path(), file_name);
				all_matching_files.insert(all_matching_files.end(), found_files.begin(),
						found_files.end());
			}
		}
	}
	return all_matching_files;
}

template<class T> void mergeData(vector<std::string> found_files,
		TFile *output_file) {
  std::cout<<"Attempting to merge files...\n";
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

		for (entry = lmd_data_vec.begin(); entry != lmd_data_vec.end(); entry++) {

			iter = merged_files.find(*entry);

			PndLmdAbstractData *lmd_data = (PndLmdAbstractData*) &(*entry);

			if (iter == merged_files.end()) {
				ret = merged_files.insert(*entry);
				//((PndLmdAbstractData*) &(*ret.first))->cloneData(*lmd_data);
			} else {
				PndLmdAbstractData *lmd_data_merge = (PndLmdAbstractData*) &(*iter);
				lmd_data_merge->add(*lmd_data);
			}
		}
	}

	output_file->cd();

	std::cout << "Merged " << merged_files.size()<< " objects from " << found_files.size() << " files!"
	      << std::endl;

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
		} else if (data_type.find("v") != std::string::npos) {
			is_valid = true;
		}
	}
	return is_valid;
}

std::string getOutputFilename(const std::string& data_type) {
	std::string output_filename("");
	if (data_type.find("a") != std::string::npos)
		output_filename = "lmd_data.root";
	else if (data_type.find("e") != std::string::npos)
		output_filename = "lmd_acc_data.root";
	else if (data_type.find("r") != std::string::npos)
		output_filename = "lmd_res_data.root";
	else if (data_type.find("v") != std::string::npos)
		output_filename = "lmd_vertex_data.root";
	return output_filename;
}

int main(int argc, char* argv[]) {
	bool is_data_path_set = false;
	bool is_filename_pattern_set = false;
	bool is_type_set = false;
	string filename_pattern("lmd_data.root");
	string dir_pattern("");
	string data_path;
	std::string data_type("");
	int c;

	while ((c = getopt(argc, argv, "hf:p:d:t:")) != -1) {
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
			case 't':
				data_type = optarg;
				is_type_set = true;
				break;
			case '?':
				if (optopt == 'f' || optopt == 'p' || optopt == 'd' || optopt == 't')
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

	std::string output_filename = getOutputFilename(data_type);

	if (is_data_path_set && is_type_set) {
		if (!is_filename_pattern_set) {
			if (data_type.find("e") != std::string::npos)
				filename_pattern = "lmd_acc_data.root";
			else if (data_type.find("r") != std::string::npos)
				filename_pattern = "lmd_res_data.root";
			else if (data_type.find("v") != std::string::npos)
				filename_pattern = "lmd_vertex_data.root";
		}

		vector<string> found_files;
		if (dir_pattern.compare("") != 0) {
			// ------ get files ------
			found_files = find_file(data_path, dir_pattern, filename_pattern);
		} else {
			found_files = find_files(data_path, filename_pattern);
		}

		std::string outfile_path = data_path + "/merge_data";
		boost::filesystem::path outdir(outfile_path);
		boost::filesystem::create_directory(outdir);

		// output file
		TFile *fmergeddata = new TFile(
				TString(outfile_path) + "/" + output_filename, "RECREATE");

		if (data_type.find("a") != std::string::npos) {
			mergeData<PndLmdAngularData>(found_files, fmergeddata);
		} else if (data_type.find("e") != std::string::npos) {
			mergeData<PndLmdAcceptance>(found_files, fmergeddata);
		} else if (data_type.find("r") != std::string::npos
				|| data_type.find("v") != std::string::npos) {
			mergeData<PndLmdHistogramData>(found_files, fmergeddata);
		}
	} else
		displayInfo();

	return 0;
}
