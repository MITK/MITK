/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MV_MODELFITCOSTFUNCTION_H
#define MV_MODELFITCOSTFUNCTION_H

#include <itkMultipleValuedCostFunction.h>
#include <itkMacro.h>

#include "mitkModelFitCostFunctionInterface.h"

#include "MitkModelFitExports.h"

namespace mitk
{

/** Base class for all model fit cost function that return a multiple cost value
 * It offers also a default implementation for the numerical computation of the
 * derivatives. Normaly you just have to (re)implement CalcMeasure().
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

    void SetSample(const SignalType &sampleSet) override;

    MeasureType GetValue(const ParametersType& parameter) const override;
    void GetDerivative (const ParametersType &parameters, DerivativeType &derivative) const override;

    unsigned int GetNumberOfValues (void) const override;
    unsigned int GetNumberOfParameters (void) const override;

    itkSetConstObjectMacro(Model, ModelBase);
    itkGetConstObjectMacro(Model, ModelBase);

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

#endif // MVModelFitCostFunction_H
