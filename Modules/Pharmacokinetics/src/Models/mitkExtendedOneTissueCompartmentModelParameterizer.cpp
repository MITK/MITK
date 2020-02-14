/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExtendedOneTissueCompartmentModelParameterizer.h"

mitk::ExtendedOneTissueCompartmentModelParameterizer::ParametersType
mitk::ExtendedOneTissueCompartmentModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(3);
  initialParameters[mitk::ExtendedOneTissueCompartmentModel::POSITION_PARAMETER_k1] = 0.5;
  initialParameters[mitk::ExtendedOneTissueCompartmentModel::POSITION_PARAMETER_k2] = 0.5;
  initialParameters[mitk::ExtendedOneTissueCompartmentModel::POSITION_PARAMETER_VB] = 0.5;

  return initialParameters;
};

mitk::ExtendedOneTissueCompartmentModelParameterizer::ExtendedOneTissueCompartmentModelParameterizer()
{
};

mitk::ExtendedOneTissueCompartmentModelParameterizer::~ExtendedOneTissueCompartmentModelParameterizer()
{
};

