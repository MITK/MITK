/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkConcreteAIFBasedModelFactory_h
#define mitkConcreteAIFBasedModelFactory_h

#include <mitkConcreteModelFactoryBase.h>
#include <mitkAIFParametrizerHelper.h>

namespace mitk
{

  /** \class ConcreteAIFBasedModelFactory
   * \brief Template factory class for AIF-based pharmacokinetic models.
   *
   * This factory creates model parameterizer instances for AIF-based models, restoring
   * the AIF values and AIF time grid from a persisted ModelFitInfo object. It serves
   * as the concrete factory for any model whose parameterizer derives from
   * AIFBasedModelParameterizerBase.
   *
   * \tparam TModelParameterizer The concrete parameterizer type for the AIF-based model.
   * \sa ConcreteModelFactoryBase, AIFBasedModelParameterizerBase
   */
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
