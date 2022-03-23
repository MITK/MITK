/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExpDecayOffsetModelParameterizer.h"

mitk::ExpDecayOffsetModelParameterizer::ParametersType
mitk::ExpDecayOffsetModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[0] = 1.0; //a
  initialParameters[1] = 1.0; //b
  initialParameters[1] = 0.0; //c

  return initialParameters;
};

mitk::ExpDecayOffsetModelParameterizer::ExpDecayOffsetModelParameterizer()
{
};

mitk::ExpDecayOffsetModelParameterizer::~ExpDecayOffsetModelParameterizer()
{
};
