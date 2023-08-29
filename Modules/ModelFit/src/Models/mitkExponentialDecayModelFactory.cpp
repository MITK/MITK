/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialDecayModelFactory.h"
#include "mitkExponentialDecayModelParameterizer.h"
#include "mitkSimpleBarrierConstraintChecker.h"

mitk::ExponentialDecayModelFactory::ExponentialDecayModelFactory()
{
};

mitk::ExponentialDecayModelFactory::~ExponentialDecayModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::ExponentialDecayModelFactory::GetDefaultInitialParameterization() const
{
  return ExponentialDecayModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ConstraintCheckerBase::Pointer mitk::ExponentialDecayModelFactory::CreateDefaultConstraints() const
{
  mitk::SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(0, 0, 0);
  constraints->SetLowerBarrier(1, 0, 0);

  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::Pointer mitk::ExponentialDecayModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  return ExponentialDecayModelParameterizer::New().GetPointer();
};
