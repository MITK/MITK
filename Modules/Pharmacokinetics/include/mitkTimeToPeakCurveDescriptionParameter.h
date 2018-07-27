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
    virtual DescriptionParameterNamesType GetDescriptionParameterName() const override;


protected:
    static const std::string PARAMETER_PEAK_NAME;
    static const std::string PARAMETER_TIME_NAME;

    TimeToPeakCurveDescriptionParameter();
    virtual ~TimeToPeakCurveDescriptionParameter();

    virtual DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;
};



}

#endif // MITKTIMETOPEAKCURVEDESCRIPTIONPARAMETER_H
