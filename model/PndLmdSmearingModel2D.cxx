/*
 * PndLmdSmearingModel2D.cxx
 *
 *  Created on: Nov 21, 2014
 *      Author: steve
 */

#include <model/PndLmdSmearingModel2D.h>

#include <cmath>

PndLmdSmearingModel2D::PndLmdSmearingModel2D() :
		max_number_of_hints(10) {
	last_found_neighbour_indices.reserve(max_number_of_hints);
}

PndLmdSmearingModel2D::~PndLmdSmearingModel2D() {
}

const std::vector<ContributorCoordinateWeight>& PndLmdSmearingModel2D::findNearestNeighbour(
		const double *x) {
	double current_distance_x(0.0);
	double current_distance_y(0.0);

	std::vector<RecoBinSmearingContributions>::const_iterator return_it =
			smearing_parameterization.end();

	//std::cout << "size of hint vector: " << last_found_neighbour_indices.size() << std::endl;
	//std::cout<<"x= "<<x[0]<<":"<<x[1]<<std::endl;
	// if everything is going right then the traversal of the recobins
	// of the model requesting the contributor list will be just in the same order
	// therefore try if the next bin after the last found one is the one!
	for (unsigned int i = 0; i < last_found_neighbour_indices.size(); ++i) {
		const RecoBinSmearingContributions &temprecobin(
				smearing_parameterization[last_found_neighbour_indices[i] + 1]);
		current_distance_x = fabs(temprecobin.reco_bin_x - x[0]);
		current_distance_y = fabs(temprecobin.reco_bin_y - x[1]);
		if (current_distance_x < search_distance_x
				&& current_distance_y < search_distance_y) {
			++(last_found_neighbour_indices[i]);
			//std::cout << "found neighbor immediately!!!" << std::endl;
			return temprecobin.contributor_coordinate_weight_list;
		}
	}

	//std::cout << "have to do it the hard way..." << std::endl;
	// if not just go through the vector to find the right one
	for (unsigned int i = 0; i < smearing_parameterization.size(); ++i) {
		const RecoBinSmearingContributions &temprecobin(
				smearing_parameterization[i]);
		current_distance_x = fabs(temprecobin.reco_bin_x - x[0]);
		current_distance_y = fabs(temprecobin.reco_bin_y - x[1]);

		if (current_distance_x < search_distance_x
				&& current_distance_y < search_distance_y) {
			if (last_found_neighbour_indices.size() == max_number_of_hints) {
				last_found_neighbour_indices.clear();
				last_found_neighbour_indices.reserve(max_number_of_hints);
			}
			last_found_neighbour_indices.push_back(i);
			return temprecobin.contributor_coordinate_weight_list;
		}
	}

	return empty_contribution_list;
}

void PndLmdSmearingModel2D::determineSearchDistance() {
	double min_dist_x(-1.0);
	double min_dist_y(-1.0);
	double binsize_x(-1.0);
	double binsize_y(-1.0);
	double current_distance_x;
	double current_distance_y;
	bool first(true);

	/* ok here we assume that the bins are equidistant...
	 *
	 */

	std::vector<RecoBinSmearingContributions>::const_iterator smear_element_it;

	std::vector<RecoBinSmearingContributions>::const_iterator first_smear_element_it =
			smearing_parameterization.begin();

	if (smearing_parameterization.size() > 0) {
		for (smear_element_it = (++smearing_parameterization.begin());
				smear_element_it != smearing_parameterization.end();
				++smear_element_it) {
			current_distance_x = fabs(
					smear_element_it->reco_bin_x - first_smear_element_it->reco_bin_x);
			current_distance_y = fabs(
					smear_element_it->reco_bin_y - first_smear_element_it->reco_bin_y);

			if (first) {
				min_dist_x = current_distance_x;
				min_dist_y = current_distance_y;
				if (current_distance_x > 0)
					binsize_x = current_distance_x;
				if (current_distance_y > 0)
					binsize_y = current_distance_y;
				first = false;
			} else {
				if (current_distance_x > 0
						&& (current_distance_x < min_dist_x || binsize_x < 0))
					binsize_x = current_distance_x;
				if (current_distance_x < min_dist_x) {
					min_dist_x = current_distance_x;
				}
				if (current_distance_y > 0
						&& (current_distance_y < min_dist_y || binsize_y < 0))
					binsize_y = current_distance_y;
				if (current_distance_y < min_dist_y) {
					min_dist_y = current_distance_y;
				}
			}
		}
	}

	search_distance_x = min_dist_x + binsize_x / 2;
	search_distance_y = min_dist_y + binsize_y / 2;

	std::cout << "determined search distance as: " << search_distance_x << " and "
			<< search_distance_y << std::endl;
}

void PndLmdSmearingModel2D::setSmearingParameterization(
		const std::vector<RecoBinSmearingContributions>& smearing_parameterization_) {
	smearing_parameterization = smearing_parameterization_;
	determineSearchDistance();
}

const std::vector<ContributorCoordinateWeight>& PndLmdSmearingModel2D::getListOfContributors(
		const double *x) {
	neighbour_index_list_lock.lock();
	const std::vector<ContributorCoordinateWeight>& result = findNearestNeighbour(
			x);
	neighbour_index_list_lock.unlock();
	return result;
}

void PndLmdSmearingModel2D::updateSmearingModel() {
}
