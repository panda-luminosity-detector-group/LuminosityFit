/*
 * AsymmetricDoubleGaussianModel1D.cxx
 *
 *  Created on: Jul 9, 2014
 *      Author: steve
 */

#include <model/AsymmetricDoubleGaussianModel1D.h>
#include <model/AsymmetricGaussianModel1D.h>

AsymmetricDoubleGaussianModel1D::AsymmetricDoubleGaussianModel1D(
    std::string name_)
    : Model1D(name_) {
  /*shared_ptr<AsymmetricGaussianModel1D> narrow_gaus(
                  new AsymmetricGaussianModel1D("narrow_gaus"));
  shared_ptr<AsymmetricGaussianModel1D> wide_gaus(
                  new AsymmetricGaussianModel1D("wide_gaus"));

  shared_ptr<Model1D> double_asymm_gaus(
                  new AdditionModel1D("double_asymm_gaus", narrow_gaus,
  wide_gaus));

  addModelToList(double_asymm_gaus);*/
  initModelParameters();
}

AsymmetricDoubleGaussianModel1D::~AsymmetricDoubleGaussianModel1D() {
  
}

void AsymmetricDoubleGaussianModel1D::initModelParameters() {
  ratio_narrow_wide_gauss =
      getModelParameterSet().addModelParameter("ratio_narrow_wide_gauss");
}

mydouble AsymmetricDoubleGaussianModel1D::eval(const double *x) const {
  // left side
  return wide_gauss->eval(x) + narrow_gauss->eval(x);
}

void AsymmetricDoubleGaussianModel1D::updateDomain() {
  double domain_low = wide_gauss->getDomainLowerBound();
  if (wide_gauss->getDomainLowerBound() > narrow_gauss->getDomainLowerBound())
    domain_low = narrow_gauss->getDomainLowerBound();
  double domain_high =
      wide_gauss->getDomainLowerBound() + wide_gauss->getDomainRange();
  if (wide_gauss->getDomainLowerBound() + wide_gauss->getDomainRange() <
      narrow_gauss->getDomainLowerBound() + narrow_gauss->getDomainRange())
    domain_high =
        narrow_gauss->getDomainLowerBound() + narrow_gauss->getDomainRange();
  setDomain(domain_low, domain_high);
}
