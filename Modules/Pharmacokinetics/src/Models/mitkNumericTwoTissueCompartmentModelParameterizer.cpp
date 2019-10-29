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

#include "mitkNumericTwoTissueCompartmentModelParameterizer.h"

mitk::NumericTwoTissueCompartmentModelParameterizer::ParametersType
mitk::NumericTwoTissueCompartmentModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(5);
  initialParameters[mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_K1] = 0.23;
  initialParameters[mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_k2] = 0.4;
  initialParameters[mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_k3] = 0.13;
  initialParameters[mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_k4] = 0.15;
  initialParameters[mitk::NumericTwoTissueCompartmentModel::POSITION_PARAMETER_VB] = 0.03;

  return initialParameters;
};

mitk::NumericTwoTissueCompartmentModelParameterizer::NumericTwoTissueCompartmentModelParameterizer()
{
};

mitk::NumericTwoTissueCompartmentModelParameterizer::~NumericTwoTissueCompartmentModelParameterizer()
{
};