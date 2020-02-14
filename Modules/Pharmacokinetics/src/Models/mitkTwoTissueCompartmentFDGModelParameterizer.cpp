/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoTissueCompartmentFDGModelParameterizer.h"

mitk::TwoTissueCompartmentFDGModelParameterizer::ParametersType
mitk::TwoTissueCompartmentFDGModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(4);
  initialParameters[mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_K1] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_k2] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_k3] = 0.5;
  initialParameters[mitk::TwoTissueCompartmentFDGModel::POSITION_PARAMETER_VB] = 0.5;

  return initialParameters;
};

mitk::TwoTissueCompartmentFDGModelParameterizer::TwoTissueCompartmentFDGModelParameterizer()
{
};

mitk::TwoTissueCompartmentFDGModelParameterizer::~TwoTissueCompartmentFDGModelParameterizer()
{
};
