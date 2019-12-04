/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOneTissueCompartmentModelParameterizer.h"

mitk::OneTissueCompartmentModelParameterizer::ParametersType
mitk::OneTissueCompartmentModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[mitk::OneTissueCompartmentModel::POSITION_PARAMETER_k1] = 0.5;
  initialParameters[mitk::OneTissueCompartmentModel::POSITION_PARAMETER_k2] = 0.5;

  return initialParameters;
};

mitk::OneTissueCompartmentModelParameterizer::OneTissueCompartmentModelParameterizer()
{
};

mitk::OneTissueCompartmentModelParameterizer::~OneTissueCompartmentModelParameterizer()
{
};
