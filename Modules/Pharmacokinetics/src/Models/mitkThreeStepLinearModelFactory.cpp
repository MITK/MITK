/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkThreeStepLinearModelFactory.h"
#include "mitkConcreteModelParameterizerBase.h"

mitk::ThreeStepLinearModelFactory::ThreeStepLinearModelFactory()
{
};

mitk::ThreeStepLinearModelFactory::~ThreeStepLinearModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::ThreeStepLinearModelFactory::GetDefaultInitialParameterization() const
{
    return ThreeStepLinearModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ModelParameterizerBase::Pointer mitk::ThreeStepLinearModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo*) const
{
  return ConcreteModelParameterizerBase<ThreeStepLinearModel>::New().GetPointer();
};
