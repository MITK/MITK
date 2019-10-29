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
