#include "PndLmdFitOptions.h"

#include <iostream>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include <boost/lexical_cast.hpp>

ClassImp(PndLmdFitOptions)

PndLmdFitOptions::PndLmdFitOptions() :
		estimator_type(LumiFit::LOG_LIKELIHOOD) {
}

boost::property_tree::ptree PndLmdFitOptions::getModelOptionsPropertyTree() const {
	boost::property_tree::ptree model_opt_ptree;

	using boost::lexical_cast;
	using boost::bad_lexical_cast;

	std::map<std::string, std::string>::const_iterator model_option;
	for (model_option = model_opt_map.begin();
			model_option != model_opt_map.end(); ++model_option) {
		bool success(false);
		// check if its an int
		try {
			if (!success) {
				int value = lexical_cast<int>(model_option->second);
				model_opt_ptree.put(model_option->first, value);
				success = true;
			}
		} catch (const bad_lexical_cast &) {
		}
		// check if its a double
		try {
			if (!success) {
				double value = lexical_cast<double>(model_option->second);
				model_opt_ptree.put(model_option->first, value);
				success = true;
			}
		} catch (const bad_lexical_cast &) {
		}
		// check if they are boolean values
		if (model_option->second.compare("true") == 0) {
			model_opt_ptree.put(model_option->first, true);
		} else if (model_option->second.compare("false") == 0) {
			model_opt_ptree.put(model_option->first, false);
		} else {
			// otherwise just take it as a string
			model_opt_ptree.put(model_option->first, model_option->second);
		}
	}

	return model_opt_ptree;
}

const EstimatorOptions& PndLmdFitOptions::getEstimatorOptions() const {
	return est_opt;
}

const std::set<std::string, ModelStructs::string_comp>& PndLmdFitOptions::getFreeParameterSet() const {
	return free_parameter_names;
}

bool PndLmdFitOptions::lessThanModelOptionsProperyTree(
		const std::map<std::string, std::string>& lhs,
		const std::map<std::string, std::string>& rhs) const {
	if (lhs < rhs)
		return true;
	else if (lhs > rhs) {
		return false;
	}
	return false;
}

bool PndLmdFitOptions::operator<(const PndLmdFitOptions &rhs) const {
	// check binary options first
	if (lessThanModelOptionsProperyTree(model_opt_map, rhs.model_opt_map))
		return true;
	else if (lessThanModelOptionsProperyTree(rhs.model_opt_map, model_opt_map))
		return false;

	if (getEstimatorOptions() < rhs.getEstimatorOptions())
		return true;
	else if (getEstimatorOptions() > rhs.getEstimatorOptions())
		return false;

	if (free_parameter_names < rhs.getFreeParameterSet())
		return true;
	else if (free_parameter_names > rhs.getFreeParameterSet())
		return false;

	return false;
}

bool PndLmdFitOptions::operator>(const PndLmdFitOptions &rhs) const {
	return (rhs < *this);
}

bool PndLmdFitOptions::operator==(const PndLmdFitOptions &fit_options) const {
	return ((*this < fit_options) == (*this > fit_options));
}

bool PndLmdFitOptions::operator!=(const PndLmdFitOptions &fit_options) const {
	return !(*this == fit_options);
}

std::ostream& operator<<(std::ostream& os,
		const PndLmdFitOptions& fit_options) {

	os << "************************************************************"
			<< std::endl;
	os << "------------------------------------------------------------"
			<< std::endl;
	os << " Model options: " << std::endl;
	os << "------------------------------------------------------------"
			<< std::endl;

	boost::property_tree::write_json(os,
			fit_options.getModelOptionsPropertyTree());

	os << std::endl
			<< "------------------------------------------------------------"
			<< std::endl;
	os << " Estimator options: " << std::endl;
	os << "------------------------------------------------------------"
			<< std::endl;
	os << "Estimator type: "
			<< LumiFit::LmdEstimatorTypeToString.at(fit_options.estimator_type)
			<< std::endl;
	os << fit_options.getEstimatorOptions();

	os << std::endl
			<< "------------------------------------------------------------"
			<< std::endl;
	os << " Free parameters: " << std::endl;
	os << "------------------------------------------------------------"
			<< std::endl;
	std::set<std::string, ModelStructs::string_comp>::const_iterator it;
	for (it = fit_options.getFreeParameterSet().begin();
			it != fit_options.getFreeParameterSet().end(); ++it) {
		os << *it << std::endl;
	}
	os << "************************************************************"
			<< std::endl;

	return os;
}
