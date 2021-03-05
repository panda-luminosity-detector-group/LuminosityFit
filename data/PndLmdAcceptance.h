/*
 * PndLmdAcceptance.h
 *
 *  Created on: Jul 4, 2012
 *      Author: steve
 */

#ifndef PNDLMDACCEPTANCE_H_
#define PNDLMDACCEPTANCE_H_

#include "PndLmdAbstractData.h"

// these includes are necessary for ROOT IO
#include "TEfficiency.h"

class TFile;

/**
 * \brief Data class used to describe the acceptance of the luminosity monitor
 * via TEfficiency objects.
 */
class PndLmdAcceptance : public PndLmdAbstractData {
private:
  // higher level acceptance objects, that are passed to the fitter etc.
  // (automatically generated later in the program)
  TEfficiency *acceptance_1d;
  TEfficiency *acceptance_2d;

  void init1DData();
  void init2DData();

public:
  PndLmdAcceptance();
  PndLmdAcceptance(const PndLmdAcceptance &lmd_acc_data_);
  ~PndLmdAcceptance();
  // getter/setter methods
  TEfficiency *getAcceptance1D() const;
  TEfficiency *getAcceptance2D() const;

  void cloneData(const PndLmdAbstractData &lmd_abs_data);
  void add(const PndLmdAbstractData &lmd_abs_data_addition);

  // acceptance filling methods
  void addData(bool is_accepted, double primary_value,
               double secondary_value = 0);

  ClassDef(PndLmdAcceptance, 2)
};

#endif /* PNDLMDACCEPTANCE_H_ */
