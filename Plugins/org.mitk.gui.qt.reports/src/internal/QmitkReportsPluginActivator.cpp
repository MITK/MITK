/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportsPluginActivator.h"
#include "QmitkReportEditor.h"
#include "QmitkReportStore.h"

#include <mitkReportsPluginConfig.h>

#if MITK_HAS_RESTAPI
#include "QmitkReportRestRoutes.h"

#include <mitkIRestServerService.h>
#include <mitkLog.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>
#endif

#include <usModuleInitialization.h>

#include <memory>

US_INITIALIZE_MODULE

namespace
{
  std::unique_ptr<QmitkReportStore> s_Store;
}

void QmitkReportsPluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkReportEditor, context)
  s_Store = std::make_unique<QmitkReportStore>();

#if MITK_HAS_RESTAPI
  // The REST server loads with the application's modules, before any plugin.
  auto* mitkContext = us::ModuleRegistry::GetModule(1)->GetModuleContext();
  const auto reference = mitkContext->GetServiceReference<mitk::IRestServerService>();
  m_RestServer = reference ? mitkContext->GetService(reference) : nullptr;

  if (m_RestServer != nullptr)
  {
    try
    {
      for (const auto& route : CreateReportRoutes(m_RestServer))
        m_RestRoutes.push_back(m_RestServer->AddRoute(route));
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "Cannot offer the reports' REST endpoints: " << e.GetDescription();
    }
  }
#endif
}

void QmitkReportsPluginActivator::stop(ctkPluginContext*)
{
#if MITK_HAS_RESTAPI
  if (m_RestServer != nullptr)
  {
    for (const auto id : m_RestRoutes)
      m_RestServer->RemoveRoute(id);
  }

  m_RestRoutes.clear();
  m_RestServer = nullptr;
#endif

  s_Store.reset();
}

QmitkReportStore* QmitkReportsPluginActivator::GetStore()
{
  return s_Store.get();
}
