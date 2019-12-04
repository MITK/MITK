/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkStandardToftsModelFactory.h"

#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkSimpleBarrierConstraintChecker.h>

mitk::ConstraintCheckerBase::Pointer
mitk::StandardToftsModelFactory::CreateDefaultConstraints() const
{
  SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_ve, 0.0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_ve, 1.0);

  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::ParametersType
mitk::StandardToftsModelFactory::GetDefaultInitialParameterization() const
{
  return StandardToftsModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::StandardToftsModelFactory::StandardToftsModelFactory()
= default;

mitk::StandardToftsModelFactory::~StandardToftsModelFactory()
= default;
