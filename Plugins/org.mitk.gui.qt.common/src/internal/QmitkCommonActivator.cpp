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

#include "QmitkCommonActivator.h"

#include <berryPlatformUI.h>
#include <mitkLogMacros.h>

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
