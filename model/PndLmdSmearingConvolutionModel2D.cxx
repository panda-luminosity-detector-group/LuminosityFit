#include "PndLmdSmearingConvolutionModel2D.h"

#include <cmath>

PndLmdSmearingConvolutionModel2D::PndLmdSmearingConvolutionModel2D(
		std::string name_, shared_ptr<Model2D> unsmeared_model_,
		shared_ptr<PndLmdSmearingModel2D> smearing_model_) :
		Model2D(name_) {

	unsmeared_model = unsmeared_model_;
	smearing_model = smearing_model_;

	addModelToList(unsmeared_model);
}

PndLmdSmearingConvolutionModel2D::~PndLmdSmearingConvolutionModel2D() {
}

void PndLmdSmearingConvolutionModel2D::initModelParameters() {
}

void PndLmdSmearingConvolutionModel2D::injectModelParameter(
		shared_ptr<ModelPar> model_param) {
	getModelParameterSet().addModelParameter(model_param);
}

double PndLmdSmearingConvolutionModel2D::eval(const double *x) const {
	double value = 0.0;

	const std::vector<ContributorCoordinateWeight> &mc_element_contributors =
			smearing_model->getListOfContributors(x);

	double xx[2];
	for (unsigned int contributor_index = 0;
			contributor_index < mc_element_contributors.size(); ++contributor_index) {
		const ContributorCoordinateWeight &mc_element_coordinate_and_weight(
				mc_element_contributors[contributor_index]);
		xx[0] = mc_element_coordinate_and_weight.bin_center_x;
		xx[1] = mc_element_coordinate_and_weight.bin_center_y;
		double integral_unsmeared_model = unsmeared_model->evaluate(xx);
		integral_unsmeared_model = integral_unsmeared_model
				* mc_element_coordinate_and_weight.area;
		value = value
				+ integral_unsmeared_model
						* mc_element_coordinate_and_weight.smear_weight;
	}
	return value;
}

void PndLmdSmearingConvolutionModel2D::updateDomain() {
	smearing_model->updateSmearingModel();
}
