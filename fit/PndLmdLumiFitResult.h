/*
 * PndLmdLumiFitResult.h
 *
 *  Created on: Jun 28, 2012
 *      Author: steve
 */

#ifndef PNDLMDLUMIFITRESULT_H_
#define PNDLMDLUMIFITRESULT_H_

#include "fit/ModelFitResult.h"

#include "TObject.h"

/**
 * \brief This class contains the fit result information and most importantly
 * the luminosity and its errors!!
 *
 * This class contains all of the information of the fit including the final
 * parameter values that were obtained by the fitting procedure.
 *
 */
class PndLmdLumiFitResult : public TObject {
private:
  double luminosity_sys_err;

  ModelFitResult model_fit_result;

public:
  PndLmdLumiFitResult();
  ~PndLmdLumiFitResult();

  double getLuminosity() const;
  double getLuminositySysError() const;
  double getLuminosityStatError() const;
  double getLuminosityError() const;

  const ModelFitResult &getModelFitResult() const;

  double getRedChiSquare() const;

  void setLuminositySysError(double luminosity_sys_err_);

  void setModelFitResult(const ModelFitResult &fit_result);

  ClassDef(PndLmdLumiFitResult, 1)
};

#endif /* PNDLMDLUMIFITRESULT_H_ */
