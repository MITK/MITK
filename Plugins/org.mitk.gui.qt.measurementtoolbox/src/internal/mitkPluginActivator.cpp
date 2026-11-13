/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkMeasurementView.h"
#include "QmitkImageStatisticsView.h"

#include <mitkMeasurementToolboxPluginConfig.h>

#if MITK_HAS_RESTAPI
#include "QmitkImageStatisticsRestRoutes.h"
#include "QmitkPlanarFigureRestRoutes.h"

#include <mitkIRestServerService.h>
#include <mitkLog.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>

#include <QThread>
#endif

ctkPluginContext* mitk::PluginActivator::m_Context = nullptr;

ctkPluginContext* mitk::PluginActivator::GetContext()
{
  return m_Context;
}

void mitk::PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMeasurementView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkImageStatisticsView, context)
  m_Context = context;

#if MITK_HAS_RESTAPI
  // The REST server loads with the application's modules, before any plugin.
  auto* mitkContext = us::ModuleRegistry::GetModule(1)->GetModuleContext();
  const auto reference = mitkContext->GetServiceReference<IRestServerService>();
  m_RestServer = reference ? mitkContext->GetService(reference) : nullptr;

  if (m_RestServer != nullptr)
  {
    m_RestRouteParent = std::make_unique<QObject>();

    try
    {
      for (const auto& route : CreateImageStatisticsRoutes(m_RestServer))
        m_RestRoutes.push_back(m_RestServer->AddRoute(route));

      for (const auto& route : CreatePlanarFigureRoutes(m_RestServer, m_RestRouteParent.get()))
        m_RestRoutes.push_back(m_RestServer->AddRoute(route));
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "Cannot offer the measurement toolbox's REST endpoints: " << e.GetDescription();
    }
  }
#endif
}

void mitk::PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

#if MITK_HAS_RESTAPI
  if (m_RestServer != nullptr)
  {
    for (const auto id : m_RestRoutes)
      m_RestServer->RemoveRoute(id);
  }

  m_RestRoutes.clear();

  // When the application ends, the framework stops its plugins in a thread of its own, from
  // which objects of the GUI thread like running timers must not be deleted. They are left
  // to the GUI thread then, which deletes them if it still handles events.
  if (m_RestRouteParent != nullptr && m_RestRouteParent->thread() != QThread::currentThread())
    m_RestRouteParent.release()->deleteLater();
  else
    m_RestRouteParent.reset();

  m_RestServer = nullptr;
#endif

  m_Context = nullptr;
}
