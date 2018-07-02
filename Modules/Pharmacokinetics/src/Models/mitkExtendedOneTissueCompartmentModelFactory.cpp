#include "mitkExtendedOneTissueCompartmentModelFactory.h"

#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkSimpleBarrierConstraintChecker.h>

mitk::ConstraintCheckerBase::Pointer
mitk::ExtendedOneTissueCompartmentModelFactory::CreateDefaultConstraints() const
{
  SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k1, 0.0);
  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k2, 0.0);
  constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_VB, 0.0);

  constraints->SetUpperBarrier (ModelType::POSITION_PARAMETER_k1, 1.0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_k2, 1.0);
  constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_VB, 1.0);

  SimpleBarrierConstraintChecker::ParameterIndexVectorType indices;
  indices.push_back(ModelType::POSITION_PARAMETER_k1);
  indices.push_back(ModelType::POSITION_PARAMETER_k2);
  indices.push_back(ModelType::POSITION_PARAMETER_VB);

  return constraints.GetPointer();
};

mitk::ModelParameterizerBase::ParametersType
mitk::ExtendedOneTissueCompartmentModelFactory::GetDefaultInitialParameterization() const
{
  return ExtendedOneTissueCompartmentModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ExtendedOneTissueCompartmentModelFactory::ExtendedOneTissueCompartmentModelFactory()
{
};

mitk::ExtendedOneTissueCompartmentModelFactory::~ExtendedOneTissueCompartmentModelFactory()
{
};

