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
#ifndef MITKTWOTISSUECOMPARTMENTFDGMODELFACTORY_H
#define MITKTWOTISSUECOMPARTMENTFDGMODELFACTORY_H

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
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:

    TwoTissueCompartmentFDGModelFactory();

    virtual ~TwoTissueCompartmentFDGModelFactory();

  private:

    //No copy constructor allowed
    TwoTissueCompartmentFDGModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKTWOTISSUECOMPARTMENTFDGMODELFACTORY_H
