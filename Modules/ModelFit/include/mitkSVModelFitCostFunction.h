/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef SV_MODELFITCOSTFUNCTION_H
#define SV_MODELFITCOSTFUNCTION_H

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

    void SetSample(const SignalType &sampleSet);

    MeasureType GetValue(const ParametersType& parameter) const;
    void GetDerivative (const ParametersType &parameters, DerivativeType &derivative) const;

    unsigned int GetNumberOfParameters (void) const;

    itkSetConstObjectMacro(Model, ModelBase);
    itkGetConstObjectMacro(Model, ModelBase);

    itkSetMacro(DerivativeStepLength, double);
    itkGetConstMacro(DerivativeStepLength, double);
 
protected:

    virtual MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const = 0;

    SVModelFitCostFunction(): m_DerivativeStepLength(1e-5)
	{
    }

    ~SVModelFitCostFunction(){}

    SignalType m_Sample;

private:
    ModelBase::ConstPointer m_Model;

    /**value (delta of parameters) used to compute the derivatives numerically*/
    double m_DerivativeStepLength;
};

}

#endif // SV_MODELFITCOSTFUNCTION_H
