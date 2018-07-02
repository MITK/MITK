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

#ifndef MITKAREAUNDERTHECURVEDESCRIPTIONPARAMETER_H
#define MITKAREAUNDERTHECURVEDESCRIPTIONPARAMETER_H

#include "mitkCurveDescriptionParameterBase.h"

namespace  mitk
{

  /** Description parameter that computes the area under the curve */
    class MITKPHARMACOKINETICS_EXPORT AreaUnderTheCurveDescriptionParameter : public mitk::CurveDescriptionParameterBase
    {
    public:
        typedef mitk::AreaUnderTheCurveDescriptionParameter Self;
        typedef CurveDescriptionParameterBase Superclass;
        typedef itk::SmartPointer< Self >                            Pointer;
        typedef itk::SmartPointer< const Self >                      ConstPointer;

        itkFactorylessNewMacro(Self);
        itkCloneMacro(Self);
        virtual DescriptionParameterNamesType GetDescriptionParameterName() const override;

    protected:
        static const std::string PARAMETER_NAME;

        AreaUnderTheCurveDescriptionParameter();
        virtual ~AreaUnderTheCurveDescriptionParameter();

        virtual DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const override;

    };
}

#endif // MITKAREAUNDERTHECURVEDESCRIPTIONPARAMETER_H
