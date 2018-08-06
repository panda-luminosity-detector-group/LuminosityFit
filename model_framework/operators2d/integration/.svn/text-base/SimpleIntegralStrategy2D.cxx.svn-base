/*
 * SimpleIntegralStrategy2D.cxx
 *
 *  Created on: Sep 26, 2014
 *      Author: steve
 */

#include <operators2d/integration/SimpleIntegralStrategy2D.h>
#include <core/Model2D.h>
#include <iostream>

SimpleIntegralStrategy2D::SimpleIntegralStrategy2D() {
	// TODO Auto-generated constructor stub

}

SimpleIntegralStrategy2D::~SimpleIntegralStrategy2D() {
	// TODO Auto-generated destructor stub
}

double SimpleIntegralStrategy2D::Integral(Model2D *model2d, double xlow,
		double xhigh, double ylow, double yhigh, double precision) {
	double result =0.0;
	double x[2];

	int cx = 100;
	int cy = 100;

	double wx = (xhigh - xlow) / cx;
	double wy = (yhigh - ylow) / cy;

	for (unsigned int ix = 0; ix < cx; ix++) {
		for (unsigned int iy = 0; iy < cy; iy++) {
			x[0] = xlow + wx * (0.5 + ix);
			x[1] = ylow + wy * (0.5 + iy);
			double tempresult = model2d->evaluate(x);
			//std::cout<<tempresult<<std::endl;
			result += tempresult;
		}
	}
	result = result * wx * wy;

	return result;
}
