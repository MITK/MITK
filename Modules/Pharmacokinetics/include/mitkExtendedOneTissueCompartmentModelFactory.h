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
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:

    ExtendedOneTissueCompartmentModelFactory();

    virtual ~ExtendedOneTissueCompartmentModelFactory();

  private:

    //No copy constructor allowed
    ExtendedOneTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif // MITKEXTENDEDONETISSUECOMPARTMENTMODELFACTORY_H
