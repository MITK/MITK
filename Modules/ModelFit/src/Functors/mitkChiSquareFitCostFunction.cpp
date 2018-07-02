#include "mitkChiSquareFitCostFunction.h"

mitk::ChiSquareFitCostFunction::MeasureType mitk::ChiSquareFitCostFunction::CalcMeasure(const ParametersType & /*parameters*/, const SignalType &signal) const
{
    MeasureType measure = 0.0;

    mitk::ModelBase::TimeGridType grid = Superclass::GetModel()->GetTimeGrid();


    double dt = 0.0;
    for(SignalType::size_type i=0; i<signal.GetSize(); ++i)
    {
        if(i==0)
        {
            dt = grid(i+1) - grid(i);
        }
        else
        {
            dt = grid(i) - grid(i-1);
        }

      double dS = sqrt( m_Sample[i] / dt );
      measure += (m_Sample[i] - signal[i]) *  (m_Sample[i] - signal[i]) / (dS * dS);
    }

    return measure;


}
