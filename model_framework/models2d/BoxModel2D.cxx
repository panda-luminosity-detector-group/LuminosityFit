#include "BoxModel2D.h"

#include <limits>

BoxModel2D::BoxModel2D(std::string name_) : Model2D(name_) {
	initModelParameters();
}

BoxModel2D::~BoxModel2D() {
	// TODO Auto-generated destructor stub
}

mydouble BoxModel2D::eval(const mydouble *x) const {
	if (x[0] >= lower_edge_var1->getValue() && x[0] <= upper_edge_var1->getValue()) {
	  if (x[1] >= lower_edge_var2->getValue() && x[1] <= upper_edge_var2->getValue()) {
			return amplitude->getValue();
	  }
	}
	return 0.0;
}

void BoxModel2D::initModelParameters() {
	amplitude = getModelParameterSet().addModelParameter("amplitude");
	amplitude->setValue(1.0);
	amplitude->setParameterFixed(true);
	lower_edge_var1 = getModelParameterSet().addModelParameter(
      "lower_edge_var1");
	upper_edge_var1 = getModelParameterSet().addModelParameter(
      "upper_edge_var1");
  lower_edge_var2 = getModelParameterSet().addModelParameter(
      "lower_edge_var2");
  upper_edge_var2 = getModelParameterSet().addModelParameter(
      "upper_edge_var2");
}

void BoxModel2D::updateDomain() {
		setVar1Domain(lower_edge_var1->getValue(), upper_edge_var1->getValue());
		setVar2Domain(lower_edge_var2->getValue(), upper_edge_var2->getValue());
}
