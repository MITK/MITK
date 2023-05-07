/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkTwoCompartmentExchangeModelFactoryBase_h
#define mitkTwoCompartmentExchangeModelFactoryBase_h

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkAIFBasedModelParameterizerBase.h"
#include "mitkSimpleBarrierConstraintChecker.h"

namespace mitk
{
  template <class TModelParameterizer>
  class TwoCompartmentExchangeModelFactoryBase : public
    mitk::ConcreteAIFBasedModelFactory< TModelParameterizer >
  {
  public:
    mitkClassMacro(TwoCompartmentExchangeModelFactoryBase,
                   ConcreteAIFBasedModelFactory< TModelParameterizer >);
    itkFactorylessNewMacro(Self);


    typedef typename Superclass::ModelType ModelType;
    typedef typename Superclass::ModelParameterizerType ModelParameterizerType;
    typedef typename Superclass::ParametersType ParametersType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override
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

    ParametersType GetDefaultInitialParameterization() const override
    {
        typename ModelParameterizerType::Pointer modelParameterizer =
          ModelParameterizerType::New();

      return modelParameterizer->GetDefaultInitialParameterization();
    };

  protected:

    TwoCompartmentExchangeModelFactoryBase()
    {
    };

    ~TwoCompartmentExchangeModelFactoryBase() override
    {
    };

  private:

    //No copy constructor allowed
    TwoCompartmentExchangeModelFactoryBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };

}
#endif
