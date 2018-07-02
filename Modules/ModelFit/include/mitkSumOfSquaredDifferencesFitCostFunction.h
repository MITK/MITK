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

#ifndef SUM_OF_SQUARED_DIFFERENCES_FITCOSTFUNCTION_H
#define SUM_OF_SQUARED_DIFFERENCES_FITCOSTFUNCTION_H

#include <mitkSVModelFitCostFunction.h>

#include "MitkModelFitExports.h"

namespace mitk
{

/** Multi valued model fit cost function that computes the squared differences between the model output and the
 * signal.
*/
class MITKMODELFIT_EXPORT SumOfSquaredDifferencesFitCostFunction : public mitk::SVModelFitCostFunction
{
public:

    typedef SumOfSquaredDifferencesFitCostFunction Self;
    typedef mitk::SVModelFitCostFunction Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);

    typedef Superclass::SignalType SignalType;

protected:

    virtual MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const;
	
    SumOfSquaredDifferencesFitCostFunction()
    {
    }

    ~SumOfSquaredDifferencesFitCostFunction(){}
};

}

#endif // SumOfSquaredDifferencesFitCostFunction_H
