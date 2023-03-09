/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkTwoTissueCompartmentFDGModelFactory_h
#define mitkTwoTissueCompartmentFDGModelFactory_h

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkTwoTissueCompartmentFDGModel.h"
#include "mitkTwoTissueCompartmentFDGModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT TwoTissueCompartmentFDGModelFactory : public
    mitk::ConcreteAIFBasedModelFactory<TwoTissueCompartmentFDGModelParameterizer>
  {
  public:
    mitkClassMacro(TwoTissueCompartmentFDGModelFactory,
                   ConcreteAIFBasedModelFactory<TwoTissueCompartmentFDGModelParameterizer>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelType ModelType;
    typedef Superclass::ModelParameterizerType ModelParameterizerType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    TwoTissueCompartmentFDGModelFactory();

    ~TwoTissueCompartmentFDGModelFactory() override;

  private:

    //No copy constructor allowed
    TwoTissueCompartmentFDGModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif
