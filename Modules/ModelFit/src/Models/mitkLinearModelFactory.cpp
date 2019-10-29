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
