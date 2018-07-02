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

    virtual MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const;

    ChiSquareFitCostFunction()
    {

    }

    ~ChiSquareFitCostFunction(){}

};
}

#endif // CHISQUAREFITCOSTFUNCTION_H
