/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKTIMETOPEAKCURVEDESCRIPTIONPARAMETER_H
#define MITKTIMETOPEAKCURVEDESCRIPTIONPARAMETER_H
#include "mitkCurveDescriptionParameterBase.h"

namespace mitk {

/** Computes the position of the first maximum of the curve. As a secondary
  * aspect it also returns the value of the curve.*/
class MITKPHARMACOKINETICS_EXPORT TimeToPeakCurveDescriptionParameter : public mitk::CurveDescriptionParameterBase
{
public:
    typedef mitk::TimeToPeakCurveDescriptionParameter Self;
    typedef CurveDescriptionParameterBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    DescriptionParameterNamesType GetDescriptionParameterName() const override;


protected:
    static const std::string PARAMETER_PEAK_NAME;
    static const std::string PARAMETER_TIME_NAME;

    TimeToPeakCurveDescriptionParameter();
    ~TimeToPeakCurveDescriptionParameter() override;

    DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;
};



}

#endif // MITKTIMETOPEAKCURVEDESCRIPTIONPARAMETER_H
