/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMitkWorkbenchIntroPlugin.h"
#include "QmitkMitkWorkbenchIntroPart.h"

#include <berryIQtStyleManager.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkVersion.h>
#include <mitkLog.h>

#include <service/cm/ctkConfigurationAdmin.h>
#include <service/cm/ctkConfiguration.h>

#include <QFileInfo>
#include <QDateTime>

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace
{
  const std::string INTRO_PREFERENCES = "org.mitk.qt.extapplicationintro";
  const std::string START_COUNT_KEY = "start count";
}

QmitkMitkWorkbenchIntroPlugin* QmitkMitkWorkbenchIntroPlugin::inst = nullptr;

QmitkMitkWorkbenchIntroPlugin::QmitkMitkWorkbenchIntroPlugin()
  : context(nullptr),
    m_StartCount(0)
{
  inst = this;
}

QmitkMitkWorkbenchIntroPlugin::~QmitkMitkWorkbenchIntroPlugin()
{
}

QmitkMitkWorkbenchIntroPlugin* QmitkMitkWorkbenchIntroPlugin::GetDefault()
{
  return inst;
}

void QmitkMitkWorkbenchIntroPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->context = context;

  auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node(INTRO_PREFERENCES);
  m_StartCount = prefs->GetInt(START_COUNT_KEY, 0) + 1;
  prefs->PutInt(START_COUNT_KEY, m_StartCount);
  prefs->Flush();

  BERRY_REGISTER_EXTENSION_CLASS(QmitkMitkWorkbenchIntroPart, context)
}

ctkPluginContext* QmitkMitkWorkbenchIntroPlugin::GetPluginContext() const
{
  return context;
}

int QmitkMitkWorkbenchIntroPlugin::GetStartCount() const
{
  return m_StartCount;
}

berry::IQtStyleManager* QmitkMitkWorkbenchIntroPlugin::GetStyleManager() const
{
  const auto reference = context->getServiceReference<berry::IQtStyleManager>();

  return reference
    ? context->getService<berry::IQtStyleManager>(reference)
    : nullptr;
}
