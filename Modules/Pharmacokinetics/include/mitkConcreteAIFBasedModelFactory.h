/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkConcreteAIFBasedModelFactory_h
#define mitkConcreteAIFBasedModelFactory_h

#include "mitkConcreteModelFactoryBase.h"
#include "mitkAIFParametrizerHelper.h"

namespace mitk
{

  template <class TModelParameterizer>
  class ConcreteAIFBasedModelFactory : public
    mitk::ConcreteModelFactoryBase<typename TModelParameterizer::ModelType>
  {
  public:
    mitkClassMacro(ConcreteAIFBasedModelFactory,
                   ConcreteModelFactoryBase<typename TModelParameterizer::ModelType>);

    typedef typename Superclass::ModelType ModelType;
    typedef TModelParameterizer ModelParameterizerType;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override
    {
      mitk::ModelParameterizerBase::Pointer result;

      typename ModelParameterizerType::Pointer modelParameterizer =
        ModelParameterizerType::New();

      modelFit::StaticParameterMap::ValueType aif = fit->staticParamMap.Get(
            ModelType::NAME_STATIC_PARAMETER_AIF);

      modelParameterizer->SetAIF(mitk::convertParameterToArray(aif));

      modelFit::StaticParameterMap::ValueType aifGrid = fit->staticParamMap.Get(
            ModelType::NAME_STATIC_PARAMETER_AIFTimeGrid);
      modelParameterizer->SetAIFTimeGrid(mitk::convertParameterToArray(aifGrid));

      result = modelParameterizer.GetPointer();

      return result;
    };

    ConcreteAIFBasedModelFactory()
    {
    };

    ~ConcreteAIFBasedModelFactory() override
    {
    };

  private:

    //No copy constructor allowed
    ConcreteAIFBasedModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif
