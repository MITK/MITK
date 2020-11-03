/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoTissueCompartmentModelParameterizer.h"

mitk::TwoTissueCompartmentModelParameterizer::ParametersType
mitk::TwoTissueCompartmentModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(5);
  initialParameters[mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_K1] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k2] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k3] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_k4] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentModel::POSITION_PARAMETER_VB] = 0.5;

  return initialParameters;
};

mitk::TwoTissueCompartmentModelParameterizer::TwoTissueCompartmentModelParameterizer()
{
};

mitk::TwoTissueCompartmentModelParameterizer::~TwoTissueCompartmentModelParameterizer()
{
};
