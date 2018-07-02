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
#ifndef MITKONETISSUECOMPARTMENTMODELFACTORY_H
#define MITKONETISSUECOMPARTMENTMODELFACTORY_H

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
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:

    OneTissueCompartmentModelFactory();

    virtual ~OneTissueCompartmentModelFactory();

  private:

    //No copy constructor allowed
    OneTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKONETISSUECOMPARTMENTMODELFACTORY_H
