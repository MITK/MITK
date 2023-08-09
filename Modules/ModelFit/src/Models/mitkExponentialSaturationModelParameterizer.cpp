/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialSaturationModelParameterizer.h"

mitk::ExponentialSaturationModelParameterizer::ParametersType
mitk::ExponentialSaturationModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(4);
  initialParameters[0] = 0.0; //bat
  initialParameters[1] = 0.0; //s0
  initialParameters[2] = 100.0; //sfin
  initialParameters[3] = 0.1; //k

  return initialParameters;
};

mitk::ExponentialSaturationModelParameterizer::ExponentialSaturationModelParameterizer()
{
};

mitk::ExponentialSaturationModelParameterizer::~ExponentialSaturationModelParameterizer()
{
};
