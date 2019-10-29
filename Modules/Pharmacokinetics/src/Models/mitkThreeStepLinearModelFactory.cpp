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
