/*
 * NumericConvolutionModel1D.cxx
 *
 *  Created on: Jan 11, 2013
 *      Author: steve
 */

#include "NumericConvolutionModel1D.h"

#include <iostream>

NumericConvolutionModel1D::NumericConvolutionModel1D(std::string name_,
		std::shared_ptr<Model1D> first_, std::shared_ptr<Model1D> second_) :
		Model1D(name_), first(first_), second(second_), divisions(300) {
	addModelToList(first);
	addModelToList(second);
}

void NumericConvolutionModel1D::initModelParameters() {

}

mydouble NumericConvolutionModel1D::eval(const mydouble *x) const {
  mydouble xx[3], val = 0.0;
  mydouble integration_range = second->getDomainRange();
	if (integration_range == 0.0) {
		return 0.0;
	}
	mydouble division_width = integration_range / 2.0 / divisions;
	mydouble xprimea, xprimem, xprimeb;
	//double checksum = 0.0;
	/*std::cout << "x0= " << x[0] << " | domain range:"
	 << second->getDomainLowerBound() << "-"
	 << second->getDomainLowerBound() + second->getDomainRange() << " | "
	 << division_width << std::endl;*/
	//integrate product of functions with simpsons rule
	//left hand part of function
	for (unsigned int i = 0; i < divisions; i++) {
		xprimea = division_width * (-1.0 * i - 1.0);
		xprimem = division_width * (-1.0 * i - 0.5);
		xprimeb = division_width * (-1.0 * i);
		xx[0] = x[0] - xprimea;
		xx[1] = x[0] - xprimem;
		xx[2] = x[0] - xprimeb;

		/*if(first->eval(&xx[0]) != first->eval(&xx[0]))
		 std::cout << "xx = "<<xx[0] << ": " << first->eval(&xx[0]);
		 if(second->eval(&xprimea) != second->eval(&xprimea))
		 std::cout << "xprimea = " << xprimea << ": " << second->eval(&xprimea) << std::endl;*/
		// simpsons formula
		val += first->eval(&xx[0]) * second->eval(&xprimea)
				+ first->eval(&xx[2]) * second->eval(&xprimeb)
				+ 4.0 * first->eval(&xx[1]) * second->eval(&xprimem);

		/*checksum += second->evaluate(&xprimea) + second->evaluate(&xprimeb)
		 + 4.0 * second->evaluate(&xprimem);*/
	}
	//std::cout << val << std::endl;
	//and right hand part
	for (unsigned int i = 0; i < divisions; i++) {
		xprimea = division_width * (1.0 * i);
		xprimem = division_width * (1.0 * i + 0.5);
		xprimeb = division_width * (1.0 * i + 1.0);
		xx[0] = x[0] - xprimea;
		xx[1] = x[0] - xprimem;
		xx[2] = x[0] - xprimeb;

		/*std::cout << xx[0] << ": " << first->evaluate(&xx[0]) << "  " << xprimea
		 << ": " << second->evaluate(&xprimea) << std::endl;*/
		// simpsons formula
		val += first->eval(&xx[0]) * second->eval(&xprimea)
				+ first->eval(&xx[2]) * second->eval(&xprimeb)
				+ 4.0 * first->eval(&xx[1]) * second->eval(&xprimem);

		/*checksum += second->evaluate(&xprimea) + second->evaluate(&xprimeb)
		 + 4.0 * second->evaluate(&xprimem);*/

	}

	/*checksum = checksum * division_width / 6.0;
	 std::cout << "checksum: " << checksum << std::endl;
	 std::cout << "x[0]: " << x[0] << " | " << second->getDomainLowerBound() << "-"
	 << second->getDomainLowerBound() + second->getDomainRange() << "  "<<second->getDomainRange()<<std::endl;
	 */

	val = val * division_width / 6.0;
	/*if(val != val)
	 std::cout << "x = " << x[0] << " -> " << val << std::endl;*/
	//std::cout<<val<<std::endl;
	return val;
}

void NumericConvolutionModel1D::updateDomain() {
	// first we need to check if user defined a domain for his models
	if (first->getDomainRange() == 0) {
		std::cout << "Warning: The domain of the model " << first->getName()
				<< " used for the convolution is not defined!" << std::endl;
	} else if (second->getDomainRange() == 0) {
		std::cout << "Warning: The domain of the model " << second->getName()
				<< " used for the convolution is not defined!" << std::endl;
	} else {
		setDomain(first->getDomain().first + second->getDomain().first,
				first->getDomain().second + second->getDomain().second);
	}
}
