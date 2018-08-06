/*
 * SimpleIntegralStrategy2D.h
 *
 *  Created on: Sep 26, 2014
 *      Author: steve
 */

#ifndef SIMPLEINTEGRALSTRATEGY2D_H_
#define SIMPLEINTEGRALSTRATEGY2D_H_

#include <operators2d/integration/IntegralStrategy2D.h>

class Model2D;

class SimpleIntegralStrategy2D: public IntegralStrategy2D {
public:
	SimpleIntegralStrategy2D();
	virtual ~SimpleIntegralStrategy2D();

	double Integral(Model2D *model1d, double xlow, double xhigh, double ylow,
			double yhigh, double precision);
};

#endif /* SIMPLEINTEGRALSTRATEGY2D_H_ */
