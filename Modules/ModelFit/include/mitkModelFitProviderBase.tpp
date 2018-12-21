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

#include <mitkModelFitProviderBase.h>
#include <mitkExceptionMacro.h>

#include <mitkExtractTimeGrid.h>
#include <mitkModelFitInfo.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

namespace mitk
{
  template<class TModelFactory>
  class ModelFitProviderBase<TModelFactory>::Impl
  {
  public:
    Impl() : m_Ranking(0), m_ReferenceFactory(TModelFactory::New())
    {
    };

    Impl(const Impl &other) = default;

    void SetRanking(int ranking)
    {
      m_Ranking = ranking;
    };

    int GetRanking() const
    {
      return m_Ranking;
    };

    us::ServiceRegistration<IModelFitProvider> m_Reg;
    int m_Ranking;
    using ModelFactoryType = TModelFactory;

    typename ModelFactoryType::Pointer m_ReferenceFactory;
  };

  template<class TModelFactory>
  ModelFitProviderBase<TModelFactory>::ModelFitProviderBase() : d()
  {
  }

  template<class TModelFactory>
  ModelFitProviderBase<TModelFactory>::~ModelFitProviderBase()
  {
    UnregisterService();
  }

  template<class TModelFactory>
  ModelFitProviderBase<TModelFactory>::ModelFitProviderBase(const ModelFitProviderBase &other) : IModelFitProvider(), d(new Impl(*other.d.get()))
  {
  }

  template<class TModelFactory>
  itk::SmartPointer<ModelFactoryBase>
    ModelFitProviderBase<TModelFactory>::GenerateFactory() const
  {
    return TModelFactory::New().GetPointer();
  };

  template<class TModelFactory>
  ModelBase::TimeGridType
    ModelFitProviderBase<TModelFactory>::GetVariableGrid(const modelFit::ModelFitInfo* fitInfo) const
  {
    if (!fitInfo)
    {
      mitkThrow() << "Error. Cannot get variable grid for model. Passed model fit info is null.";
    }

    if (!fitInfo->inputImage.IsNotNull())
    {
      mitkThrow() << "Error. Cannot get variable grid for model. Passed model fit info has no input image.";
    }
    return mitk::ExtractTimeGrid(fitInfo->inputImage);
  };


  template<class TModelFactory>
  us::ServiceRegistration<IModelFitProvider>
    ModelFitProviderBase<TModelFactory>::RegisterService(us::ModuleContext *context)
  {
    if (d)
      return d->m_Reg;

    if (context == nullptr)
    {
      context = us::GetModuleContext();
    }

    d.reset(new Impl());

    us::ServiceProperties props = this->GetServiceProperties();
    d->m_Reg = context->RegisterService<IModelFitProvider>(this, props);
    return d->m_Reg;
  }

  template<class TModelFactory>
  void
    ModelFitProviderBase<TModelFactory>::UnregisterService()
  {
    try
    {
      d->m_Reg.Unregister();
    }
    catch (const std::exception &)
    {
    }
  }

  template<class TModelFactory>
  us::ServiceProperties
  ModelFitProviderBase<TModelFactory>::GetServiceProperties() const
  {
    us::ServiceProperties result;

    result[IModelFitProvider::PROP_MODEL_CLASS_ID()] = this->d->m_ReferenceFactory->GetClassID();
    result[IModelFitProvider::PROP_MODEL_TYPE()] = this->d->m_ReferenceFactory->GetModelType();
    result[us::ServiceConstants::SERVICE_RANKING()] = this->GetRanking();
    return result;
  }

  template<class TModelFactory>
  void
  ModelFitProviderBase<TModelFactory>::SetRanking(int ranking) { d->SetRanking(ranking); }

  template<class TModelFactory>
  int
  ModelFitProviderBase<TModelFactory>::GetRanking() const { return d->GetRanking(); }

}
