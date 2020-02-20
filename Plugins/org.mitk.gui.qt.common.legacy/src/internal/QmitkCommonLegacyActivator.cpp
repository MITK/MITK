/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCommonLegacyActivator.h"
#include <berryPlatformUI.h>
#include <mitkLogMacros.h>

ctkPluginContext* QmitkCommonLegacyActivator::m_Context = nullptr;

void
QmitkCommonLegacyActivator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)

  if(berry::PlatformUI::IsWorkbenchRunning())
  {
    m_FunctionalityCoordinator.Start();
    m_Context = context;
  }
  else
  {
    MITK_ERROR << "BlueBerry Workbench not running!";
  }
}

void
QmitkCommonLegacyActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  m_FunctionalityCoordinator.Stop();
  m_Context = nullptr;
}

ctkPluginContext*QmitkCommonLegacyActivator::GetContext()
{
  return m_Context;
}
