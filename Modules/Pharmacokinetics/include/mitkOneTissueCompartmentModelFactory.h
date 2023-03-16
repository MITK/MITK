/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkOneTissueCompartmentModelFactory_h
#define mitkOneTissueCompartmentModelFactory_h

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkOneTissueCompartmentModel.h"
#include "mitkOneTissueCompartmentModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT OneTissueCompartmentModelFactory : public
    mitk::ConcreteAIFBasedModelFactory<OneTissueCompartmentModelParameterizer>
  {
  public:
    mitkClassMacro(OneTissueCompartmentModelFactory,
                   ConcreteAIFBasedModelFactory<OneTissueCompartmentModelParameterizer>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelType ModelType;
    typedef Superclass::ModelParameterizerType ModelParameterizerType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    OneTissueCompartmentModelFactory();

    ~OneTissueCompartmentModelFactory() override;

  private:

    //No copy constructor allowed
    OneTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif
