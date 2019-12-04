/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGenericParamModelFactory.h"
#include "mitkGenericParamModelParameterizer.h"

mitk::GenericParamModelFactory::GenericParamModelFactory()
= default;

mitk::GenericParamModelFactory::~GenericParamModelFactory()
= default;

mitk::ModelParameterizerBase::ParametersType
mitk::GenericParamModelFactory::GetDefaultInitialParameterization() const
{
  return GenericParamModelParameterizer::New()->GetDefaultInitialParameterization();
};


mitk::ModelParameterizerBase::Pointer
mitk::GenericParamModelFactory::DoCreateParameterizer(
  const mitk::modelFit::ModelFitInfo* fit) const
{
  mitk::ModelParameterizerBase::Pointer result;

  GenericParamModelParameterizer::Pointer modelParameterizer =
    GenericParamModelParameterizer::New();

  auto paramCount = fit->staticParamMap.Get(
    GenericParamModel::NAME_STATIC_PARAMETER_number);
  modelParameterizer->SetNumberOfParameters(paramCount[0]);

  result = modelParameterizer.GetPointer();

  return result;
};
