/*
 * PndLmdSignalBackgroundModel.cxx
 *
 *  Created on: Dec 19, 2014
 *      Author: steve
 */

#include <model/PndLmdSignalBackgroundModel1D.h>

#include <iostream>

PndLmdSignalBackgroundModel1D::PndLmdSignalBackgroundModel1D(std::string name_,
		std::shared_ptr<Model1D> signal_, std::shared_ptr<Model1D> background_) :
		Model1D(name_), signal(signal_), background(background_) {
	initModelParameters();
	addModelToList(signal);
	addModelToList(background);
}

PndLmdSignalBackgroundModel1D::~PndLmdSignalBackgroundModel1D() {
}

void PndLmdSignalBackgroundModel1D::initModelParameters() {
	signal_fraction = getModelParameterSet().addModelParameter("signal_fraction");
	signal_fraction->setValue(1.0);
	background_fraction = getModelParameterSet().addModelParameter(
			"background_fraction");
	background_fraction->setValue(0.0);
}

mydouble PndLmdSignalBackgroundModel1D::eval(const mydouble *x) const {
	return signal_fraction->getValue() * signal->eval(x)
			+ background_fraction->getValue() * background->eval(x);
}

void PndLmdSignalBackgroundModel1D::updateDomain() {
	// first we need to check if user defined a domain for his models
	if (signal->getDomainRange() == 0) {
		std::cout << "Warning: The domain of the model " << signal->getName()
				<< " used for the addition is not defined!" << std::endl;
	} else if (background->getDomainRange() == 0) {
		std::cout << "Warning: The domain of the model " << background->getName()
				<< " used for the addition is not defined!" << std::endl;
	} else {
		setDomain(
				std::min(signal->getDomain().first, background->getDomain().first),
				std::max(signal->getDomain().second, background->getDomain().second));
	}
}
