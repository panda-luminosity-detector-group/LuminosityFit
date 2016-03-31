/*
 * PndLmdDPMMTModel1D.h
 *
 *  Created on: Dec 19, 2012
 *      Author: steve
 */

#ifndef PNDLMDDPMMTMODEL1D_H_
#define PNDLMDDPMMTMODEL1D_H_

#include "core/Model1D.h"
#include "LumiFitStructs.h"

/**
 * \brief This class is the base of lumi models such as #PndLmdLumiModelROOT and #PndLmdLumiModelRooFit which can be fitted to #PndLmdData objects.
 *
 * The reason for a model base class is a common model description in all different fit implementations (ROOT and RooFit etc).
 * The DPM model description can be found in the #evaluate() function.
 *
 * Also this class contains all the necessary handles for the only degree of freedom, the acceptance description (see #PndLmdAcceptance),
 * which is required for applying an acceptance correction. The acceptance description can of course differ depending on its binning, data size, etc.
 * Hence usually more than one model instance can be active.
 */
class PndLmdDPMMTModel1D: public Model1D {
	protected:
		// constants
		double pi;
		double hbarc2;
		double alpha;
		double M;
		double alpha_squared_4pi;
		double one_over_16pi_hbarc2;

		/**
		 * References to the values of the parameters used in the DPM description
		 * of the cross section that are constants or are actually parameters in
		 * the fit
		 */
		//strictly fixed parameters
		shared_ptr<ModelPar> p_lab;
		shared_ptr<ModelPar> luminosity;
		shared_ptr<ModelPar> E_lab;
		shared_ptr<ModelPar> S;
		shared_ptr<ModelPar> pcm2;
		shared_ptr<ModelPar> gamma;
		shared_ptr<ModelPar> beta;
		shared_ptr<ModelPar> beta_lab_cms;
		//possibly free fit parameters
		shared_ptr<ModelPar> sigma_tot;
		shared_ptr<ModelPar> b;
		shared_ptr<ModelPar> rho;
		shared_ptr<ModelPar> A1;
		shared_ptr<ModelPar> A2;
		shared_ptr<ModelPar> A3;
		shared_ptr<ModelPar> T1;
		shared_ptr<ModelPar> T2;

		// function pointer used to switch between different algorithms for interpolation
		typedef double (PndLmdDPMMTModel1D::*function)(const double *x) const;

		function model_func;

		/**
		 *  initializes the above parameters of the DPM cross section that are
		 *  absolutely fixed (so not dependent on the beam momentum for example)
		 */
		void init();

		void updateDomainFromPars(double *par);

	public:
		LumiFit::DPMElasticParts elastic_type;

		/**
		 * In the constructor that creates a fully defined pure signal cross section
		 * model of LMD
		 * @param name_ is the name of the model. Make sure that it is unique.
		 */
		PndLmdDPMMTModel1D(std::string name_,
				LumiFit::DPMElasticParts elastic_type_);

		~PndLmdDPMMTModel1D();

		virtual void initModelParameters();

		/** @returns parameter b of DPM model cross section */
		double getB() const;
		/** @returns parameter rho of DPM model cross section */
		double getRho() const;
		/** @returns parameter sigma total of DPM model cross section */
		double getSigmaTotal() const;

		double getDelta(const double t) const;

		double getProtonDipoleFormFactor(const double t) const;

		double getRawCoulombPart(const double *x) const;

		double getRawInterferencePart(const double *x) const;

		double getRawHadronicPart(const double *x) const;

		double getRawRhoBSigtotHadronicPart(const double *x) const;

		double getRawFullElastic(const double *x) const;

		double getRawRhoBSigtotFullElastic(const double *x) const;

		virtual double eval(const double *x) const;

		virtual void updateDomain();
};

#endif /* PNDLMDDPMMTMODEL1D_H_ */
