/*
 * IntegralStrategySR1D.cxx
 *
 *  Created on: Apr 30, 2013
 *      Author: steve
 */

#include "IntegralStrategySR1D.h"
#include "core/Model1D.h"

#include <cmath>

IntegralStrategySR1D::IntegralStrategySR1D() {
  // TODO Auto-generated constructor stub
}

IntegralStrategySR1D::~IntegralStrategySR1D() {
  // TODO Auto-generated destructor stub
}

mydouble IntegralStrategySR1D::Integral(Model1D *model1d, mydouble xlow,
                                        mydouble xhigh, mydouble precision) {
  // ok increase the number of divisions until precision is reached
  unsigned int divisions = 0;
  mydouble current_precision = 0.0;
  mydouble val = 0.0;
  mydouble last_value = 0.0;
  do {
    divisions++;
    val = 0.0;
    mydouble x[3];
    mydouble division_width = (xhigh - xlow) / divisions;
    for (unsigned int i = 0; i < divisions; i++) {
      x[0] = xlow + division_width * (1.0 * i);
      x[1] = xlow + division_width * (1.0 * i + 0.5);
      x[2] = xlow + division_width * (1.0 * i + 1.0);

      // simpsons formula
      val += model1d->evaluate(&x[0]) + 4.0 * model1d->evaluate(&x[1]) +
             model1d->evaluate(&x[2]);
      // std::cout << evaluate(&x[0]) << " " << evaluate(&x[1]) << " "
      //    << evaluate(&x[2]) << std::endl;
    }
    val *= division_width / 6.0;
    if (val == 0.0)
      break;
    // std::cout << "integral = " << val << std::endl;
    current_precision = fabs((val - last_value) / val);
    last_value = val;
    // std::cout<<"using "<<divisions<<" divisions: "<<current_precision << " <
    // " << precision <<std::endl;
  } while (current_precision > precision / 1e1);
  // std::cout << "xrange: "<<xlow<<" - "<<xhigh<<"  returning integral = " <<
  // val << std::endl;
  return val;
}
