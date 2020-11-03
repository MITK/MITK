/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKEXTENDEDONETISSUECOMPARTMENTMODELFACTORY_H
#define MITKEXTENDEDONETISSUECOMPARTMENTMODELFACTORY_H

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkExtendedOneTissueCompartmentModel.h"
#include "mitkExtendedOneTissueCompartmentModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT ExtendedOneTissueCompartmentModelFactory : public
    mitk::ConcreteAIFBasedModelFactory< ExtendedOneTissueCompartmentModelParameterizer>
  {
  public:
    mitkClassMacro(ExtendedOneTissueCompartmentModelFactory,
                   ConcreteAIFBasedModelFactory<ExtendedOneTissueCompartmentModelParameterizer>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelType ModelType;
    typedef Superclass::ModelParameterizerType ModelParameterizerType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    ExtendedOneTissueCompartmentModelFactory();

    ~ExtendedOneTissueCompartmentModelFactory() override;

  private:

    //No copy constructor allowed
    ExtendedOneTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif // MITKEXTENDEDONETISSUECOMPARTMENTMODELFACTORY_H
