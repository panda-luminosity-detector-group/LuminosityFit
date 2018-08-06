#include "SmearingConvolutionModel2D.h"

#include <cmath>
#include <iostream>

SmearingConvolutionModel2D::SmearingConvolutionModel2D(std::string name_,
		std::shared_ptr<Model2D> first_, std::shared_ptr<Model2D> second_) :
		Model2D(name_), first(first_), second(second_), divisions_var1(100), divisions_var2(
				100) {
	addModelToList(first);
	addModelToList(second);
}

SmearingConvolutionModel2D::~SmearingConvolutionModel2D() {
}

void SmearingConvolutionModel2D::initModelParameters() {

}

mydouble SmearingConvolutionModel2D::eval(const mydouble *x) const {
	// x[0]/x[1] are the reconstructed values
	//std::cout<<x[0]<<" : "<<x[1]<<std::endl;
  mydouble value = 0.0;
	// first divide the domain of the first model (that should be smeared) into subintervals
  mydouble division_width_var1 = first->getVar1DomainRange() / divisions_var1;
  mydouble range_low_var1 = first->getVar1DomainLowerBound();
  mydouble division_width_var2 = first->getVar2DomainRange() / divisions_var2;
	mydouble range_low_var2 = first->getVar2DomainLowerBound();

	mydouble interval_center[2];
	mydouble smear_interval_center[2];

	std::vector<DataStructs::DimensionRange> temp_range_first;
	DataStructs::DimensionRange dr;
	temp_range_first.push_back(dr);
	temp_range_first.push_back(dr);

	//get domain of the second model at the evaluation point x and only scan in surrounding
	second->evaluate(x);
	unsigned int i1min = (unsigned int) (0.9
			* (x[0] + second->getVar1DomainLowerBound()
					- first->getVar1DomainLowerBound()) / division_width_var1);
	unsigned int i1max = (unsigned int) (1.1
			* (x[0] + second->getVar1DomainLowerBound() + second->getVar1DomainRange()
					- first->getVar1DomainLowerBound()) / division_width_var1) + 1;
	unsigned int i2min = (unsigned int) (0.9
			* (x[1] + second->getVar2DomainLowerBound()
					- first->getVar2DomainLowerBound()) / division_width_var2);
	unsigned int i2max = (unsigned int) (1.1
			* (x[1] + second->getVar2DomainLowerBound() + second->getVar2DomainRange()
					- first->getVar2DomainLowerBound()) / division_width_var2) + 1;

	if (i1max > divisions_var1)
		i1max = divisions_var1;
	if (i2max > divisions_var2)
		i2max = divisions_var2;

	// now loop over these subinvervals and integrate the to be smeared model in this subinterval region
	// and integrate the smearing model in the shifted region to the evaluation point
	for (unsigned int i1 = i1min; i1 < i1max; i1++) {
		for (unsigned int i2 = i2min; i2 < i2max; i2++) {
			temp_range_first[0].range_low = range_low_var1 + division_width_var1 * i1;
			temp_range_first[0].range_high = temp_range_first[0].range_low
					+ division_width_var1;
			temp_range_first[1].range_low = range_low_var2 + division_width_var2 * i2;
			temp_range_first[1].range_high = temp_range_first[1].range_low
					+ division_width_var2;

			// check if the second model is in range for the current subinterval
			// to reach the reconstructed value x[0]
			interval_center[0] = temp_range_first[0].getDimensionMean();
			interval_center[1] = temp_range_first[1].getDimensionMean();

			second->evaluate(interval_center); // this is to get the correct parametrization models activated
			//second->getModelParameterSet().printInfo();

			/*temp_range_second[0].range_low = x[0] - interval_center[0]
			 - 0.5 * division_width_var1;
			 temp_range_second[0].range_high = temp_range_second[0].range_low
			 + division_width_var1;
			 temp_range_second[1].range_low = x[1] - interval_center[1]
			 - 0.5 * division_width_var2;
			 temp_range_second[1].range_high = temp_range_second[1].range_low
			 + division_width_var2;*/

			// if not in range skip this interval
			if (second->getVar1DomainRange() < division_width_var1
					|| second->getVar2DomainRange() < division_width_var2)
				continue;
			/* if (second->getVar1DomainLowerBound() + second->getVar1DomainRange()
			 < temp_range_second[0].range_low
			 || second->getVar1DomainLowerBound()
			 > temp_range_second[0].range_high)
			 continue;
			 if (second->getVar2DomainLowerBound() + second->getVar2DomainRange()
			 < temp_range_second[1].range_low
			 || second->getVar2DomainLowerBound()
			 > temp_range_second[1].range_high)
			 continue;*/

			/*std::cout << second->getVar1DomainLowerBound() << "-"
			 << second->getVar1DomainLowerBound() + second->getVar1DomainRange() << std::endl;
			 std::cout << second->getVar2DomainLowerBound() << "-"
			 << second->getVar2DomainLowerBound() + second->getVar2DomainRange() << std::endl;
			 std::cout<<"--------------"<<std::endl;
			 std::cout << "theta, phi= " << x[0] << ", " << x[1] << std::endl;
			 std::cout<<"--------------"<<std::endl;
			 std::cout << temp_range_second[0].range_low << "-"
			 << temp_range_second[0].range_high << std::endl;
			 std::cout << temp_range_second[1].range_low << "-"
			 << temp_range_second[1].range_high << std::endl;

			 std::cout<<"--------------"<<std::endl;

			 std::cout << temp_range_first[0].range_low << "-"
			 << temp_range_first[0].range_high << std::endl;
			 std::cout << temp_range_first[1].range_low << "-"
			 << temp_range_first[1].range_high << std::endl;*/

			//std::cout<<x[0]<< " "<<interval_low<<" "<<smear_interval_low<<std::endl;
			// integrate second (smearing) function in that smear interval range
			//double int_second = second->Integral(temp_range_second, 1e-3);
			// integrate second (smearing) function in that smear interval range
			//double int_first = first->Integral(temp_range_first, 1e-3);
			smear_interval_center[0] = x[0] - interval_center[0];
			smear_interval_center[1] = x[1] - interval_center[1];

			/*std::cout << "gaus integral: " << int_second << std::endl;
			 std::cout << "  dpmmodel integral: " << int_first << std::endl;*/

			mydouble intfirst = first->evaluate(interval_center);
			if (intfirst != intfirst) {
				/*std::cout << interval_center[0] << " : " << interval_center[1] << " => "
				 << intfirst << std::endl;*/
				intfirst = 0.0;
			}
			mydouble intsecond = second->eval(smear_interval_center);
			if (intsecond != intsecond) {
				/*second->getModelParameterSet().printInfo();
				 std::cout << smear_interval_center[0] << " : "
				 << smear_interval_center[1] << " => " << intsecond << std::endl;
				 std::cout<<x[0]<<" : "<<x[1]<<std::endl;*/
				intsecond = 0.0;
			}

			value += intfirst * intsecond;
		}
	}
	if (value != 0.0)
		value = value * division_width_var1 * division_width_var2;

	return value;
}

void SmearingConvolutionModel2D::updateDomain() {
	// first we need to check if user defined a domain for his models
	/*	if (first->getVar1DomainRange() == 0.0
	 || first->getVar2DomainRange() == 0.0) {
	 std::cout << "Warning: The domain of the model " << first->getName()
	 << " used for the convolution is not defined!" << std::endl;
	 } else if (second->getVar1DomainRange() == 0.0
	 || second->getVar2DomainRange() == 0.0) {
	 std::cout << "Warning: The domain of the model " << second->getName()
	 << " used for the convolution is not defined!" << std::endl;
	 } else {
	 setVar1Domain(first->getDomain().first + second->getDomain().first,
	 first->getDomain().second + second->getDomain().second);
	 }*/
}
