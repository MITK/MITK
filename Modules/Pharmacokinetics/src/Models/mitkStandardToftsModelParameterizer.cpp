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

#include "mitkStandardToftsModelParameterizer.h"

mitk::StandardToftsModelParameterizer::ParametersType
mitk::StandardToftsModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[mitk::StandardToftsModel::POSITION_PARAMETER_Ktrans] = 15;
  initialParameters[mitk::StandardToftsModel::POSITION_PARAMETER_ve] = 0.5;

  return initialParameters;
};

mitk::StandardToftsModelParameterizer::StandardToftsModelParameterizer()
{
};

mitk::StandardToftsModelParameterizer::~StandardToftsModelParameterizer()
{
};
