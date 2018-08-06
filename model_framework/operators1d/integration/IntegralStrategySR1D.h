/*
 * IntegralStrategySR1D.h
 *
 *  Created on: Apr 30, 2013
 *      Author: steve
 */

#ifndef INTEGRALSTRATEGYSR1D_H_
#define INTEGRALSTRATEGYSR1D_H_

#include "IntegralStrategy1D.h"

class IntegralStrategySR1D: public IntegralStrategy1D {
public:
	IntegralStrategySR1D();
	virtual ~IntegralStrategySR1D();

	mydouble Integral(Model1D *model1d, mydouble xlow, mydouble xhigh, mydouble precision);
};

#endif /* INTEGRALSTRATEGYSR1D_H_ */
