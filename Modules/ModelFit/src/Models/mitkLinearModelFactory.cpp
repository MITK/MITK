/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLinearModelFactory.h"
#include "mitkConcreteModelParameterizerBase.h"

mitk::LinearModelFactory::LinearModelFactory()
{
};

mitk::LinearModelFactory::~LinearModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::LinearModelFactory::GetDefaultInitialParameterization() const
{
  return ConcreteModelParameterizerBase<LinearModel>::New()->GetDefaultInitialParameterization();
};

mitk::ModelParameterizerBase::Pointer mitk::LinearModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  return ConcreteModelParameterizerBase<LinearModel>::New().GetPointer();
};
