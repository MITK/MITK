/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExtendedToftsModelFactory.h"

#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkSimpleBarrierConstraintChecker.h>

mitk::ConstraintCheckerBase::Pointer
mitk::ExtendedToftsModelFactory::CreateDefaultConstraints() const
{
  SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_ve, 0.0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_ve, 1.0);
  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_vp, 0.0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_vp, 1.0);

  SimpleBarrierConstraintChecker::ParameterIndexVectorType indices;
  indices.push_back(ModelType::POSITION_PARAMETER_ve);
  indices.push_back(ModelType::POSITION_PARAMETER_vp);

  constraints->SetUpperSumBarrier(indices, 1.0);

  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::ParametersType
mitk::ExtendedToftsModelFactory::GetDefaultInitialParameterization() const
{
  return ExtendedToftsModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ExtendedToftsModelFactory::ExtendedToftsModelFactory()
= default;

mitk::ExtendedToftsModelFactory::~ExtendedToftsModelFactory()
= default;
