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
