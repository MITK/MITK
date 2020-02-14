/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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



