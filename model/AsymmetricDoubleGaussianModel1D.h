#ifndef ASYMMETRICDOUBLEGAUSSIANMODEL1D_H_
#define ASYMMETRICDOUBLEGAUSSIANMODEL1D_H_

#include <core/Model1D.h>

class AsymmetricGaussianModel1D;

class AsymmetricDoubleGaussianModel1D: public Model1D {
	shared_ptr<AsymmetricGaussianModel1D> wide_gauss;
	shared_ptr<AsymmetricGaussianModel1D> narrow_gauss;

	shared_ptr<ModelPar> ratio_narrow_wide_gauss;

public:
	AsymmetricDoubleGaussianModel1D(std::string name_);
	virtual ~AsymmetricDoubleGaussianModel1D();

	void initModelParameters();

	mydouble eval(const double *x) const;

	void updateDomain();
};

#endif /* ASYMMETRICDOUBLEGAUSSIANMODEL1D_H_ */
