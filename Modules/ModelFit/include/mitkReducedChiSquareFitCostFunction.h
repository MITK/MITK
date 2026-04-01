/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkReducedChiSquareFitCostFunction_h
#define mitkReducedChiSquareFitCostFunction_h

#include <mitkSVModelFitCostFunction.h>

#include <MitkModelFitExports.h>

namespace mitk
{

    /**
     * \class ReducedChiSquareFitCostFunction
     * \brief Single-valued cost function that computes the reduced chi-square statistic.
     *
     * The reduced chi-square normalizes the chi-square value by the number of degrees
     * of freedom (number of data points minus number of fitted parameters), providing
     * a measure of goodness-of-fit that is comparable across fits with different numbers
     * of parameters.
     *
     * \sa SVModelFitCostFunction, ChiSquareFitCostFunction
     */
    class MITKMODELFIT_EXPORT ReducedChiSquareFitCostFunction : public mitk::SVModelFitCostFunction
    {
    public:
        typedef ReducedChiSquareFitCostFunction Self;
        typedef mitk::SVModelFitCostFunction Superclass;
        typedef itk::SmartPointer< Self >   Pointer;
        typedef itk::SmartPointer< const Self >                      ConstPointer;
        itkNewMacro(Self);

        typedef Superclass::SignalType SignalType;

    protected:

        MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const override;

        ReducedChiSquareFitCostFunction()
        {

        }

        ~ReducedChiSquareFitCostFunction() override{}

    };

}
#endif
