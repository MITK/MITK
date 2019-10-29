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

#include "mitkExtendedToftsModelParameterizer.h"

mitk::ExtendedToftsModelParameterizer::ParametersType
mitk::ExtendedToftsModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(3);
  initialParameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_Ktrans] = 15;
  initialParameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_ve] = 0.5;
  initialParameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_vp] = 0.05;

  return initialParameters;
};

mitk::ExtendedToftsModelParameterizer::ExtendedToftsModelParameterizer()
{
};

mitk::ExtendedToftsModelParameterizer::~ExtendedToftsModelParameterizer()
{
};
