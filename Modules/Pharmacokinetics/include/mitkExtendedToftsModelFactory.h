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
#ifndef MITKEXTENDEDTOFTSMODELFACTORY_H
#define MITKEXTENDEDTOFTSMODELFACTORY_H

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkExtendedToftsModel.h"
#include "mitkExtendedToftsModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT ExtendedToftsModelFactory : public
    mitk::ConcreteAIFBasedModelFactory<ExtendedToftsModelParameterizer>
  {
  public:
    mitkClassMacro(ExtendedToftsModelFactory,
                   ConcreteAIFBasedModelFactory<ExtendedToftsModelParameterizer>);
    itkFactorylessNewMacro(Self);

    using ModelType = Superclass::ModelType;
    using ModelParameterizerType = Superclass::ModelParameterizerType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;
    virtual ParametersType GetDefaultInitialParameterization() const override;

  protected:

    ExtendedToftsModelFactory();

    virtual ~ExtendedToftsModelFactory() override;

  private:

    //No copy constructor allowed
    ExtendedToftsModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKEXTENDEDTOFTSMODELFACTORY_H
