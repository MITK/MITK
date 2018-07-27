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
#ifndef MITKSTANDARDTOFTSMODELFACTORY_H
#define MITKSTANDARDTOFTSMODELFACTORY_H

#include "mitkConcreteAIFBasedModelFactory.h"
#include "mitkStandardToftsModel.h"
#include "mitkStandardToftsModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT StandardToftsModelFactory : public
    mitk::ConcreteAIFBasedModelFactory<StandardToftsModelParameterizer>
  {
  public:
    mitkClassMacro(StandardToftsModelFactory, ConcreteAIFBasedModelFactory<StandardToftsModelParameterizer>);
    itkFactorylessNewMacro(Self);

    using ModelType = Superclass::ModelType;
    using ModelParameterizerType = Superclass::ModelParameterizerType;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;
    virtual ParametersType GetDefaultInitialParameterization() const override;

  protected:

    StandardToftsModelFactory();

    virtual ~StandardToftsModelFactory() override;

  private:

    //No copy constructor allowed
    StandardToftsModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKSTANDARDTOFTSMODELFACTORY_H
