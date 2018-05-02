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

#include <QmitkDataStorageInspectorProviderBase.h>
#include <mitkExceptionMacro.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usPrototypeServiceFactory.h>

  template<class TInspector>
  class QmitkDataStorageInspectorProviderBase<TInspector>::Impl
  {
  public:
    Impl(const std::string& id, const std::string& displayName, const std::string& desc) : m_Ranking(0), m_ID(id), m_DisplayName(displayName), m_Desc(desc)
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

    us::ServiceRegistration<mitk::IDataStorageInspectorProvider> m_Reg;
    int m_Ranking;
    std::string m_ID;
    std::string m_DisplayName;
    std::string m_Desc;
  };

  template<class TInspector>
  QmitkDataStorageInspectorProviderBase<TInspector>::QmitkDataStorageInspectorProviderBase(const std::string& id) : QmitkDataStorageInspectorProviderBase(id, id)
  {
  }

  template<class TInspector>
  QmitkDataStorageInspectorProviderBase<TInspector>::QmitkDataStorageInspectorProviderBase(const std::string& id, const std::string& displayName, const std::string& desc)
  {
    d.reset(new Impl(id, displayName, desc));
    RegisterService();
  }

  template<class TInspector>
  QmitkDataStorageInspectorProviderBase<TInspector>::~QmitkDataStorageInspectorProviderBase()
  {
    UnregisterService();
  }

  template<class TInspector>
  QmitkDataStorageInspectorProviderBase<TInspector>::QmitkDataStorageInspectorProviderBase(const QmitkDataStorageInspectorProviderBase &other) : IDataStorageInspectorProvider(), d(new Impl(*other.d.get()))
  {
  }

  template<class TInspector>
  QmitkAbstractDataStorageInspector*
    QmitkDataStorageInspectorProviderBase<TInspector>::CreateInspector() const
  {
    return new TInspector;
  };

  template<class TInspector>
  std::string
    QmitkDataStorageInspectorProviderBase<TInspector>::GetInspectorID() const
  {
    return d->m_ID;
  };

  template<class TInspector>
  std::string
    QmitkDataStorageInspectorProviderBase<TInspector>::GetInspectorDisplayName() const
  {
    return d->m_DisplayName;
  };
  
  template<class TInspector>
  std::string
    QmitkDataStorageInspectorProviderBase<TInspector>::GetInspectorDescription() const
  {
    return d->m_Desc;
  };

  template<class TInspector>
  us::ServiceRegistration<mitk::IDataStorageInspectorProvider> 
    QmitkDataStorageInspectorProviderBase<TInspector>::RegisterService(us::ModuleContext *context)
  {
    if (d->m_Reg)
      return d->m_Reg;

    if (context == nullptr)
    {
      context = us::GetModuleContext();
    }

    us::ServiceProperties props = this->GetServiceProperties();
    d->m_Reg = context->RegisterService<IDataStorageInspectorProvider>(this, props);
    return d->m_Reg;
  }

  template<class TInspector>
  void
    QmitkDataStorageInspectorProviderBase<TInspector>::UnregisterService()
  {
    try
    {
      d->m_Reg.Unregister();
    }
    catch (const std::exception &)
    {
    }
  }

  template<class TInspector>
  us::ServiceProperties
  QmitkDataStorageInspectorProviderBase<TInspector>::GetServiceProperties() const
  {
    us::ServiceProperties result;

    result[IDataStorageInspectorProvider::PROP_INSPECTOR_ID()] = this->d->m_ID;
    result[us::ServiceConstants::SERVICE_RANKING()] = this->GetRanking();
    return result;
  }

  template<class TInspector>
  void
  QmitkDataStorageInspectorProviderBase<TInspector>::SetRanking(int ranking) { d->SetRanking(ranking); }

  template<class TInspector>
  int
  QmitkDataStorageInspectorProviderBase<TInspector>::GetRanking() const { return d->GetRanking(); }
