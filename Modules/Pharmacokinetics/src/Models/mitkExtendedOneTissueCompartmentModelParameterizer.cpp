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

