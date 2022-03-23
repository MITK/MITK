/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExpDecayOffsetModelFactory.h"
#include "mitkExpDecayOffsetModelParameterizer.h"
#include "mitkSimpleBarrierConstraintChecker.h"

mitk::ExpDecayOffsetModelFactory::ExpDecayOffsetModelFactory()
{
};

mitk::ExpDecayOffsetModelFactory::~ExpDecayOffsetModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::ExpDecayOffsetModelFactory::GetDefaultInitialParameterization() const
{
  return ExpDecayOffsetModelParameterizer::New()->GetDefaultInitialParameterization();
};


mitk::ModelParameterizerBase::Pointer mitk::ExpDecayOffsetModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  return ExpDecayOffsetModelParameterizer::New().GetPointer();
};
