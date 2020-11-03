/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkT2DecayModelParameterizer.h"

mitk::T2DecayModelParameterizer::ParametersType
mitk::T2DecayModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[0] = 10; //M0
  initialParameters[1] = 1000; //T2

  return initialParameters;
};

mitk::T2DecayModelParameterizer::T2DecayModelParameterizer()
{
};

mitk::T2DecayModelParameterizer::~T2DecayModelParameterizer()
{
};
