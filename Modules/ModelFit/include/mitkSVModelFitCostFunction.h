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

#include "mitkModelFitCostFunctionInterface.h"

#include "MitkModelFitExports.h"

namespace mitk
{

/** Base class for all model fit cost function that return a singel cost value*/
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

    void SetSample(const SignalType &sampleSet) override;

    MeasureType GetValue(const ParametersType& parameter) const override;
    void GetDerivative (const ParametersType &parameters, DerivativeType &derivative) const override;

    unsigned int GetNumberOfParameters (void) const override;

    itkSetConstObjectMacro(Model, ModelBase);
    itkGetConstObjectMacro(Model, ModelBase);

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
