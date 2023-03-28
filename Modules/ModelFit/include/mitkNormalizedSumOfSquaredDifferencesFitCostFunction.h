/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNormalizedSumOfSquaredDifferencesFitCostFunction_h
#define mitkNormalizedSumOfSquaredDifferencesFitCostFunction_h

#include <mitkSVModelFitCostFunction.h>

#include "MitkModelFitExports.h"

namespace mitk
{

/** Multi valued model fit cost function that computes the squared differences between the model output and the
 * signal.
*/
class MITKMODELFIT_EXPORT NormalizedSumOfSquaredDifferencesFitCostFunction : public mitk::SVModelFitCostFunction
{
public:

    typedef NormalizedSumOfSquaredDifferencesFitCostFunction Self;
    typedef mitk::SVModelFitCostFunction Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);

    typedef Superclass::SignalType SignalType;

protected:

    MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const override;

    NormalizedSumOfSquaredDifferencesFitCostFunction()
    {
    }

    ~NormalizedSumOfSquaredDifferencesFitCostFunction() override{}
};

}

#endif
