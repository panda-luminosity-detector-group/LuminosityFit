/*
 * PndLmdBackgroundModel1D.h
 *
 *  Created on: Dec 19, 2014
 *      Author: steve
 */

#ifndef PNDLMDBACKGROUNDMODEL1D_H_
#define PNDLMDBACKGROUNDMODEL1D_H_

#include <core/Model1D.h>

class PndLmdBackgroundModel1D: public Model1D {
	shared_ptr<Model1D> polynomial_model;
public:
	PndLmdBackgroundModel1D();
	virtual ~PndLmdBackgroundModel1D();

	void initModelParameters();

	double eval(const double *x) const;

	void updateDomain();
};

#endif /* PNDLMDBACKGROUNDMODEL1D_H_ */
