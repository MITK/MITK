/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMVModelFitCostFunction_h
#define mitkMVModelFitCostFunction_h

#include <itkMultipleValuedCostFunction.h>
#include <itkMacro.h>

#include <mitkModelFitCostFunctionInterface.h>

#include <MitkModelFitExports.h>

namespace mitk
{

/**
 * \class MVModelFitCostFunction
 * \brief Base class for all model fit cost functions that return a vector of cost values.
 *
 * Inherits from itk::MultipleValuedCostFunction and ModelFitCostFunctionInterface.
 * This class provides a default numerical derivative computation. Subclasses only
 * need to implement CalcMeasure() to define the specific cost metric.
 *
 * \sa SVModelFitCostFunction, SquaredDifferencesFitCostFunction, MVConstrainedCostFunctionDecorator
 */
class MITKMODELFIT_EXPORT MVModelFitCostFunction : public itk::MultipleValuedCostFunction, public ModelFitCostFunctionInterface
{
public:

    typedef MVModelFitCostFunction Self;
    typedef itk::MultipleValuedCostFunction Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef ModelFitCostFunctionInterface::SignalType SignalType;
    typedef Superclass::MeasureType MeasureType;
    typedef Superclass::DerivativeType DerivativeType;

    /**
     * \brief Sets the observed sample signal.
     * \param[in] sampleSet The sample signal array.
     */
    void SetSample(const SignalType &sampleSet) override;

    /**
     * \brief Computes the cost value vector for the given parameters.
     * \param[in] parameter The model parameters to evaluate.
     * \return A vector of cost values (one per time point).
     */
    MeasureType GetValue(const ParametersType& parameter) const override;

    /**
     * \brief Computes the Jacobian matrix numerically via finite differences.
     * \param[in] parameters The parameters at which the derivative is evaluated.
     * \param[out] derivative The computed Jacobian matrix.
     */
    void GetDerivative (const ParametersType &parameters, DerivativeType &derivative) const override;

    /**
     * \brief Returns the number of values (residuals) returned by GetValue().
     * \return The number of cost values (equals the sample signal size).
     */
    unsigned int GetNumberOfValues (void) const override;

    /**
     * \brief Returns the number of model parameters.
     * \return The number of parameters of the associated model.
     */
    unsigned int GetNumberOfParameters (void) const override;

    mitkOverrideSetConstObjectMacro(Model, ModelBase);
    mitkOverrideGetConstObjectMacro(Model, ModelBase);

    itkSetMacro(DerivativeStepLength, double);
    itkGetConstMacro(DerivativeStepLength, double);

protected:

    virtual MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const = 0;

    MVModelFitCostFunction() : m_DerivativeStepLength(1e-5)
    {
    }

    ~MVModelFitCostFunction() override{}

    SignalType m_Sample;

private:
    ModelBase::ConstPointer m_Model;

    /**value (delta of parameters) used to compute the derivatives numerically*/
    double m_DerivativeStepLength;
};

}

#endif
