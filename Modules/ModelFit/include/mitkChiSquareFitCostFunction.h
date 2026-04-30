/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkChiSquareFitCostFunction_h
#define mitkChiSquareFitCostFunction_h

#include <mitkSVModelFitCostFunction.h>

#include <MitkModelFitExports.h>


namespace mitk
{
/**
 * \class ChiSquareFitCostFunction
 * \brief Single-valued cost function that computes the chi-square statistic.
 *
 * This cost function computes the chi-square goodness-of-fit measure, which is
 * appropriate for data from radioactive decay processes (e.g. PET, SPECT) where
 * the variance of each measurement equals its expected value.
 *
 * \note This cost function assumes Poisson-distributed data (radioactive decays).
 *
 * \sa SVModelFitCostFunction, ReducedChiSquareFitCostFunction
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

#endif
