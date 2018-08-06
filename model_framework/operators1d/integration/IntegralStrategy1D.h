/*
 * IntegralStrategy1D.h
 *
 *  Created on: Apr 30, 2013
 *      Author: steve
 */

#ifndef INTEGRALSTRATEGY1D_H_
#define INTEGRALSTRATEGY1D_H_

#include "ProjectWideSettings.h"

class Model1D;

class IntegralStrategy1D {
public:
	IntegralStrategy1D();
	virtual ~IntegralStrategy1D();

	virtual mydouble Integral(Model1D *model1d, mydouble xlow, mydouble xhigh,
			mydouble precision) =0;
};

#endif /* INTEGRALSTRATEGY1D_H_ */
