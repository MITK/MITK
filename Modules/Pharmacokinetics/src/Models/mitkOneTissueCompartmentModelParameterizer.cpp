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
