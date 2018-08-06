/*
 * Model.h
 *
 *  Created on: Jan 8, 2013
 *      Author: steve
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "ModelParameterHandler.h"

#include "fit/data/DataStructs.h"

#include <set>
#include <string>

/**
 * This defines the abstract structure of a Model.
 * In principle a model should generate the model parameters itself, and be able
 * to return the set via #getModelParameterSet() to be initialized by the user
 * or some parameterization which is both external.
 */
class Model {
private:
	std::string name;

	unsigned int dimension;

	/**
	 * The parameter set of the model.
	 */
	ModelParameterHandler model_par_handler;

	/**
	 * If this model is constructed from other models via some operation etc.
	 * they are listed here.
	 */
	std::vector<std::shared_ptr<Model> > submodel_list;

protected:
	// certain operations are totally equivalent for the 1d and 2d case
	// define these here
	mydouble multiply(std::shared_ptr<Model> m1, std::shared_ptr<Model> m2, const mydouble *x) const;

	mydouble add(std::shared_ptr<Model> m1, std::shared_ptr<Model> m2, const mydouble *x) const;

	void addModelToList(std::shared_ptr<Model> model);

public:
	/**
	 * see #Model description
	 */
	Model(std::string name_, unsigned int dimension_);
	virtual ~Model();

	void executeParametrizationModels(const mydouble *x);

	/**
	 * Called by the #evaluate() function and actually does an evaluation of
	 * this model with the given parameters. Has to be overwritten by any
	 * derived class.
	 */
	virtual mydouble eval(const mydouble *x) const =0;

	virtual std::pair<mydouble, mydouble> getUncertaincy(const mydouble *x) const;

	/**
	 * This function will be called by the fitter when an evaluation at a certain
	 * position is made. It calls all of the ParametrizationModels that modify
	 * the parameter set connected to this set. Afterwards the normal evaluation
	 * function of the Model is called (see #eval())
	 */
	mydouble evaluate(const mydouble *x);

	virtual mydouble Integral(const std::vector<DataStructs::DimensionRange> &ranges
			, mydouble precision) =0;

	/**
	 * This function has to be overwritten by the user and has to define and
	 * initialize all of the parameters which are required by this model.
	 * IMPORTANT: This function has to be called inside the constructor of the
	 * derived class, otherwise the parameter allocation/handling fails!
	 */
	virtual void initModelParameters() =0;

	void reinit();

	int init();

	/**
	 * Returns the number of dimensions of this Model
	 *
	 * @returns the number of dimensions of this model
	 */
	unsigned int getDimension() const;

	/**
	 * Get name of this model. The model name can only be set by the constructor!
	 * The unique naming of models is important for automated parameter handling.
	 *
	 * @returns the name of this model
	 */
	std::string getName() const;

	ModelParSet& getModelParameterSet();

	ModelParameterHandler& getModelParameterHandler();

	void updateDomainForModelWithName(const std::string& name);

	/**
	 * This method updates the model parameters that are set via parametrizations
	 * and domains that are not fixed (free) by the ones in the array pars. This
	 * function has to be called once after each minimizer iteration step (see
	 * #ModelFitInterface).
	 */
	void updateModel();

	void setParametersUnmodified();

	/**
	 * Interface for updating the domain of the model, which is useful/ a
	 * requirement for the numerical convolution process.
	 * This method should update the domain depending on the parameter set.
	 * Of course strictly mathematically speaking the domain of a model should not
	 * depend on the parameters of the model (if all of the parameters are within
	 * their domains). However for integration speed purposes the domain can be
	 * shortened to some smaller area, while the outside of this domain is
	 * numerically compatible with zero. The user is forced to overwrite this
	 * method to an appropriate domain (see #GaussianModel1D::updateDomain()).
	 * Most likely a change in the parameters, will also change the domain, hence
	 * this method will be called once the parameters have changes.
	 */
	virtual void updateDomain() =0;
};

#endif /* MODEL_H_ */
