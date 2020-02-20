/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    StandardToftsModelFactory();

    ~StandardToftsModelFactory() override;

  private:

    //No copy constructor allowed
    StandardToftsModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKSTANDARDTOFTSMODELFACTORY_H
