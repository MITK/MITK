/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialSaturationModelFactory.h"
#include "mitkExponentialSaturationModelParameterizer.h"
#include "mitkSimpleBarrierConstraintChecker.h"

mitk::ExponentialSaturationModelFactory::ExponentialSaturationModelFactory()
{
};

mitk::ExponentialSaturationModelFactory::~ExponentialSaturationModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::ExponentialSaturationModelFactory::GetDefaultInitialParameterization() const
{
  return ExponentialSaturationModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ConstraintCheckerBase::Pointer mitk::ExponentialSaturationModelFactory::CreateDefaultConstraints() const
{
  mitk::SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(0, 0, 0);
  constraints->SetLowerBarrier(1, 0, 0);

  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::Pointer mitk::ExponentialSaturationModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  return ExponentialSaturationModelParameterizer::New().GetPointer();
};
