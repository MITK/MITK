/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSVModelFitCostFunction_h
#define mitkSVModelFitCostFunction_h

#include <itkSingleValuedCostFunction.h>
#include <itkMacro.h>

#include <mitkModelFitCostFunctionInterface.h>

#include <MitkModelFitExports.h>

namespace mitk
{

/**
 * \class SVModelFitCostFunction
 * \brief Base class for all model fit cost functions that return a single scalar cost value.
 *
 * Inherits from itk::SingleValuedCostFunction and ModelFitCostFunctionInterface.
 * Subclasses must implement CalcMeasure() to define the specific cost metric.
 * Provides a default numerical derivative computation via finite differences.
 *
 * \sa MVModelFitCostFunction, ModelFitCostFunctionInterface, SumOfSquaredDifferencesFitCostFunction
 */
class MITKMODELFIT_EXPORT SVModelFitCostFunction : public itk::SingleValuedCostFunction, public ModelFitCostFunctionInterface
{
public:

    typedef SVModelFitCostFunction Self;
    typedef itk::SingleValuedCostFunction Superclass;
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
     * \brief Computes the scalar cost value for the given parameters.
     * \param[in] parameter The model parameters to evaluate.
     * \return The scalar cost measure value.
     */
    MeasureType GetValue(const ParametersType& parameter) const override;

    /**
     * \brief Computes the derivative of the cost function numerically via finite differences.
     * \param[in] parameters The parameters at which the derivative is evaluated.
     * \param[out] derivative The computed derivative vector.
     */
    void GetDerivative (const ParametersType &parameters, DerivativeType &derivative) const override;

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

    SVModelFitCostFunction(): m_DerivativeStepLength(1e-5)
	{
    }

    ~SVModelFitCostFunction() override{}

    SignalType m_Sample;

private:
    ModelBase::ConstPointer m_Model;

    /**value (delta of parameters) used to compute the derivatives numerically*/
    double m_DerivativeStepLength;
};

}

#endif
