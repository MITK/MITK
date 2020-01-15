/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestModelFactory.h"
#include "mitkConcreteModelParameterizerBase.h"

mitk::TestModelFactory::TestModelFactory()
{
};

mitk::TestModelFactory::~TestModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::TestModelFactory::GetDefaultInitialParameterization() const
{
  ParametersType dummy;
  return dummy;
};

mitk::ModelParameterizerBase::Pointer mitk::TestModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  mitk::ModelParameterizerBase::Pointer dummy;
  return dummy;
};
