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

#include <mitkLog.h>
#include <mitkRestHelpers.h>
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
  m_RestRoutes = std::make_unique<mitk::RestRouteRegistration>();

  if (m_RestRoutes->GetServer() != nullptr)
  {
    try
    {
      m_RestRoutes->Add(CreateReportRoutes(m_RestRoutes->GetServer()));
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
  m_RestRoutes.reset();
#endif

  s_Store.reset();
}

QmitkReportStore* QmitkReportsPluginActivator::GetStore()
{
  return s_Store.get();
}
