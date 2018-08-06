/*
 * ProductModel1D.h
 *
 *  Created on: Jan 10, 2013
 *      Author: steve
 */

#ifndef PRODUCTMODEL1D_H_
#define PRODUCTMODEL1D_H_

#include "core/Model1D.h"

class ProductModel1D: public Model1D {
private:
	std::shared_ptr<Model1D> first, second;
public:
	ProductModel1D(std::string name_, std::shared_ptr<Model1D> first_,
			std::shared_ptr<Model1D> second_);

	void initModelParameters();

	mydouble eval(const mydouble *x) const;

	void updateDomain();

	virtual std::pair<mydouble, mydouble> getUncertaincy(const mydouble *x) const;
};

#endif /* PRODUCTMODEL1D_H_ */
