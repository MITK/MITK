/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialDecayModelParameterizer.h"

mitk::ExponentialDecayModelParameterizer::ParametersType
mitk::ExponentialDecayModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[0] = 10; //y0
  initialParameters[1] = 1000; //lambda

  return initialParameters;
};

mitk::ExponentialDecayModelParameterizer::ExponentialDecayModelParameterizer()
{
};

mitk::ExponentialDecayModelParameterizer::~ExponentialDecayModelParameterizer()
{
};
