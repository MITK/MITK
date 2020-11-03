/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCommonActivator.h"

#include <berryPlatformUI.h>
#include <mitkLogMacros.h>
#include "QmitkNodeSelectionPreferencePage.h"

QmitkCommonActivator* QmitkCommonActivator::m_Instance = nullptr;
ctkPluginContext* QmitkCommonActivator::m_Context = nullptr;

ctkPluginContext* QmitkCommonActivator::GetContext()
{
  return m_Context;
}

QmitkCommonActivator* QmitkCommonActivator::GetInstance()
{
  return m_Instance;
}

berry::IPreferencesService* QmitkCommonActivator::GetPreferencesService()
{
  return m_PrefServiceTracker->getService();
}

void
QmitkCommonActivator::start(ctkPluginContext* context)
{
  this->m_Instance = this;
  this->m_Context = context;
  this->m_PrefServiceTracker.reset(new ctkServiceTracker<berry::IPreferencesService*>(context));

  if(berry::PlatformUI::IsWorkbenchRunning())
  {
    m_ViewCoordinator.reset(new QmitkViewCoordinator);
    m_ViewCoordinator->Start();
  }
  else
  {
    MITK_ERROR << "BlueBerry Workbench not running!";
  }

  BERRY_REGISTER_EXTENSION_CLASS(QmitkNodeSelectionPreferencePage, context)
}

void
QmitkCommonActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  m_ViewCoordinator->Stop();
  m_ViewCoordinator.reset();

  this->m_PrefServiceTracker.reset();

  this->m_Context = nullptr;
  this->m_Instance = nullptr;
}
