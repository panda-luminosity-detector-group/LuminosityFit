/*
 * PndLmdLumiModelFitter.cxx
 *
 *  Created on: Jan 21, 2013
 *      Author: steve
 */

#include "ROOTMinimizer.h"

#include "Math/Factory.h"
#include "Math/Functor.h"
#include "TMath.h"

ROOTMinimizer::ROOTMinimizer(int type) {
  std::cout << "Initializing Minuit Minimizer..." << std::endl;

  // min = ROOT::Math::Factory::CreateMinimizer("GSLMultiMin", "BFGS2");
  min = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
  // set tolerance , etc...
  if (type == 0) {
    min->SetMaxFunctionCalls(100);
    min->SetMaxIterations(1);
  } else {
    min->SetMaxFunctionCalls(100000);
    min->SetMaxIterations(100);
  }
  // min->SetTolerance(1.0); // default: 0.01
  // min->SetPrecision(1e-8);
  min->SetPrintLevel(5);
}

ROOTMinimizer::~ROOTMinimizer() {
  
}

void ROOTMinimizer::increaseFunctionCallLimit() {
  min->SetMaxFunctionCalls(min->MaxFunctionCalls() * 2);
}

const ROOT::Math::Minimizer *ROOTMinimizer::getROOTMinimizer() const {
  return min;
}

double ROOTMinimizer::root_func_wrapper(const double *x) {
  unsigned int size(control_parameter->getParameterList().size());
  mydouble xtemp[size];
  for (unsigned int i = 0; i < size; ++i) {
    xtemp[i] = (mydouble)x[i];
  }
  return (double)control_parameter->evaluate(xtemp);
}

ModelFitResult ROOTMinimizer::createModelFitResult() const {
  ModelFitResult fit_result;

  for (unsigned int i = 0; i < min->NDim(); i++) {
    std::string model_name;
    std::string param_name;

    unsigned int colon_index = min->VariableName(i).find(":");
    model_name = min->VariableName(i).substr(0, colon_index);
    param_name = min->VariableName(i).substr(
        colon_index + 1, min->VariableName(i).size() - colon_index);

    fit_result.addFitParameter(std::make_pair(model_name, param_name),
                               min->X()[i], min->Errors()[i]);
  }

  return fit_result;
}

int ROOTMinimizer::minimize() {
  std::cout << "Setting up fit..." << std::endl;
  min->Clear();
  // create function wrapper for minmizer  a IMultiGenFunction type
  std::cout << "Number of free parameters in fit: "
            << control_parameter->getParameterList().size() << std::endl;
  ROOT::Math::Functor fc(this, &ROOTMinimizer::root_func_wrapper,
                         control_parameter->getParameterList().size());
  min->SetFunction(fc);

  // Set the free variables to be minimized!
  for (unsigned int i = 0; i < control_parameter->getParameterList().size();
       i++) {
    double stepsize =
        TMath::Abs(0.2 * control_parameter->getParameterList()[i].value);
    if (0.0 == control_parameter->getParameterList()[i].value)
      stepsize = 0.1;
    min->SetVariable(i,
                     control_parameter->getParameterList()[i].name.first + ":" +
                         control_parameter->getParameterList()[i].name.second,
                     control_parameter->getParameterList()[i].value, stepsize);
  }
  std::cout << "Finished setting up fit!" << std::endl;

  std::cout << "Performing fit..." << std::endl;
  int error_code = 0;
  if (!min->Minimize())
    error_code = 1;
  std::cout << "Fit done!" << std::endl;
  return error_code;
}
