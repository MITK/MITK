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

#include "mitkT2DecayModelParameterizer.h"

mitk::T2DecayModelParameterizer::ParametersType
mitk::T2DecayModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[0] = 10; //M0
  initialParameters[1] = 1000; //T2

  return initialParameters;
};

mitk::T2DecayModelParameterizer::T2DecayModelParameterizer()
{
};

mitk::T2DecayModelParameterizer::~T2DecayModelParameterizer()
{
};
