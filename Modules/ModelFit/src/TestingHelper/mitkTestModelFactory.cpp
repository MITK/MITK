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
