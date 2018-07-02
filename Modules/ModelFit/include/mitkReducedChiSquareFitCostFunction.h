#ifndef REDUCED_CHI_SQUARE_FITCOSTFUNCTION_H
#define REDUCED_CHI_SQUARE_FITCOSTFUNCTION_H

#include <mitkSVModelFitCostFunction.h>

#include "MitkModelFitExports.h"

namespace mitk
{

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

        virtual MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const;

        ReducedChiSquareFitCostFunction()
        {

        }

        ~ReducedChiSquareFitCostFunction(){}

    };

}
#endif // REDUCEDCHISQUAREFITCOSTFUNCTION_H
