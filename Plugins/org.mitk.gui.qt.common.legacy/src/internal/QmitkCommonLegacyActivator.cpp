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
