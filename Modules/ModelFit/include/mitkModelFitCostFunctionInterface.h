/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFitCostFunctionInterface_h
#define mitkModelFitCostFunctionInterface_h

#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{
/**
 * \class ModelFitCostFunctionInterface
 * \brief Interface class that is implemented by all cost functions used in model fitting.
 *
 * This interface defines the common API for cost functions that evaluate how well
 * a model fits observed data. Implementations compute a cost/measure based on the
 * difference between the model signal and the observed sample signal.
 *
 * \sa SVModelFitCostFunction, MVModelFitCostFunction
 */
class MITKMODELFIT_EXPORT ModelFitCostFunctionInterface
{
public:

    typedef ModelFitCostFunctionInterface Self;

    /** \brief Type representing the observed signal values. */
    typedef itk::Array<double> SignalType;

    /**
     * \brief Sets the observed sample signal that the model will be fitted against.
     * \param[in] sampleSet The observed signal values.
     */
    virtual void SetSample(const SignalType &sampleSet) = 0;

    /**
     * \brief Sets the model instance to be used for cost evaluation.
     * \param[in] model Pointer to the model to evaluate.
     */
    virtual void SetModel(const ModelBase* model) = 0;

    /**
     * \brief Returns the model instance currently used for cost evaluation.
     * \return Const pointer to the current model.
     */
    virtual const ModelBase* GetModel() const = 0;

protected:

    ModelFitCostFunctionInterface()
    {
    }

    virtual ~ModelFitCostFunctionInterface()
    {}

private:

    ModelFitCostFunctionInterface(const ModelFitCostFunctionInterface& source);
    void operator=(const ModelFitCostFunctionInterface&);  //purposely not implemented

};

}

#endif
