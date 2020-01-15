/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    ExtendedToftsModelFactory();

    ~ExtendedToftsModelFactory() override;

  private:

    //No copy constructor allowed
    ExtendedToftsModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKEXTENDEDTOFTSMODELFACTORY_H
