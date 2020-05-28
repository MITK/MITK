/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkTwoStepLinearModelFactory.h"
#include "mitkConcreteModelParameterizerBase.h"

mitk::TwoStepLinearModelFactory::TwoStepLinearModelFactory()
{
};

mitk::TwoStepLinearModelFactory::~TwoStepLinearModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::TwoStepLinearModelFactory::GetDefaultInitialParameterization() const
{
    return TwoStepLinearModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ModelParameterizerBase::Pointer mitk::TwoStepLinearModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  return ConcreteModelParameterizerBase<TwoStepLinearModel>::New().GetPointer();
};
