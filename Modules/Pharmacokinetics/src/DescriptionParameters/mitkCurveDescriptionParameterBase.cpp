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

#include "mitkCurveDescriptionParameterBase.h"
#include "itkMacro.h"

#include <algorithm>


mitk::CurveDescriptionParameterBase::CurveDescriptionParameterBase()
{
}

mitk::CurveDescriptionParameterBase::~CurveDescriptionParameterBase()
{
}

mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType mitk::CurveDescriptionParameterBase::GetCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const
{
  if(curve.GetSize() != grid.GetSize())
  {
    itkExceptionMacro("Cannot calculate Curve Description Parameter. Signal and Time Grid do not match!");
  }

  DescriptionParameterResultsType parameter = ComputeCurveDescriptionParameter(curve, grid);

  return parameter;
}



