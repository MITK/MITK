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

#include "mitkT2DecayModelFactory.h"
#include "mitkT2DecayModelParameterizer.h"
#include "mitkSimpleBarrierConstraintChecker.h"

mitk::T2DecayModelFactory::T2DecayModelFactory()
{
};

mitk::T2DecayModelFactory::~T2DecayModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::T2DecayModelFactory::GetDefaultInitialParameterization() const
{
  return T2DecayModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ConstraintCheckerBase::Pointer mitk::T2DecayModelFactory::CreateDefaultConstraints() const
{
  mitk::SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(0, 0, 0);
  constraints->SetLowerBarrier(1, 0, 0);

  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::Pointer mitk::T2DecayModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* /*fit*/) const
{
  return T2DecayModelParameterizer::New().GetPointer();
};
