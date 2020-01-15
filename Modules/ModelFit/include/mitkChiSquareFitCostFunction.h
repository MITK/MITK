/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CHI_SQUARE_FITCOSTFUNCTION_H
#define CHI_SQUARE_FITCOSTFUNCTION_H

#include <mitkSVModelFitCostFunction.h>

#include "MitkModelFitExports.h"


namespace mitk
{
/** Multi valued model fit cost function that computes the Chi square. NOTE: This is only for Data from Radioactive Decays (e.g. PET, SPECT)
*/
class MITKMODELFIT_EXPORT ChiSquareFitCostFunction : public mitk::SVModelFitCostFunction
{
public:

    typedef ChiSquareFitCostFunction Self;
    typedef mitk::SVModelFitCostFunction Superclass;
    typedef itk::SmartPointer< Self >   Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;
    itkNewMacro(Self);

    typedef Superclass::SignalType SignalType;

protected:

    MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const override;

    ChiSquareFitCostFunction()
    {

    }

    ~ChiSquareFitCostFunction() override{}

};
}

#endif // CHISQUAREFITCOSTFUNCTION_H
