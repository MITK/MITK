/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoTissueCompartmentFDGModelFactory.h"

#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkSimpleBarrierConstraintChecker.h>

mitk::ConstraintCheckerBase::Pointer
mitk::TwoTissueCompartmentFDGModelFactory::CreateDefaultConstraints() const
{
  SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_K1, 0.0);
  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k2, 0, 0);
  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k3, 0, 0);
  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_VB, 0, 0);

  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_K1, 1.0, 0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_k2, 1.0, 0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_k3, 1.0, 0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_VB, 1, 0);


  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::ParametersType
mitk::TwoTissueCompartmentFDGModelFactory::GetDefaultInitialParameterization() const
{
  return TwoTissueCompartmentFDGModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::TwoTissueCompartmentFDGModelFactory::TwoTissueCompartmentFDGModelFactory()
{
};

mitk::TwoTissueCompartmentFDGModelFactory::~TwoTissueCompartmentFDGModelFactory()
{
};
