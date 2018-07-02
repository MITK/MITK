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
#ifndef MITKTWOTISSUECOMPARTMENTFACTORYBASE_H
#define MITKTWOTISSUECOMPARTMENTFACTORYBASE_H

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkAIFBasedModelParameterizerBase.h"
#include "mitkSimpleBarrierConstraintChecker.h"

namespace mitk
{
  template <class TModelParameterizer>
  class TwoTissueCompartmentModelFactoryBase : public
    mitk::ConcreteAIFBasedModelFactory< TModelParameterizer >
  {
  public:
    mitkClassMacro(TwoTissueCompartmentModelFactoryBase,
                   ConcreteAIFBasedModelFactory< TModelParameterizer >);
    itkFactorylessNewMacro(Self);

    typedef typename Superclass::ModelType ModelType;
    typedef typename Superclass::ModelParameterizerType ModelParameterizerType;
    typedef typename Superclass::ParametersType ParametersType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override
    {
      SimpleBarrierConstraintChecker::Pointer constraints = SimpleBarrierConstraintChecker::New();

      /**@TODO: Mit Charlie klaren ob es eine sinnvolle default Einstellung gibt.*/
      constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_K1, 0, 0);
      constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k2, 0, 0);
      constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k3, 0, 0);
      constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_k4, 0, 0);
      constraints->SetLowerBarrier(ModelType::POSITION_PARAMETER_VB, 0, 0);
      constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_VB, 1, 0);

      constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_K1, 1.0, 0);
      constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_k2, 1.0, 0);
      constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_k3, 1.0, 0);
      constraints->SetUpperBarrier(ModelType::POSITION_PARAMETER_k4, 1.0, 0);


      return constraints.GetPointer();
    };

    virtual ParametersType GetDefaultInitialParameterization() const
    {
        typename ModelParameterizerType::Pointer modelParameterizer =
          ModelParameterizerType::New();

      return modelParameterizer->GetDefaultInitialParameterization();
    };

  protected:

    TwoTissueCompartmentModelFactoryBase()
    {
    };

    virtual ~TwoTissueCompartmentModelFactoryBase()
    {
    };

  private:

    //No copy constructor allowed
    TwoTissueCompartmentModelFactoryBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };

}
#endif // MITKTWOCOMPARTMENTEXCHANGEMODELFACTORY_H
