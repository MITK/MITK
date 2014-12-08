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

#include <QtPlugin>

#include <berryPlatformUI.h>

#include <mitkLogMacros.h>

void
QmitkCommonLegacyActivator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)

  if(berry::PlatformUI::IsWorkbenchRunning())
  {
    m_FunctionalityCoordinator = QmitkFunctionalityCoordinator::Pointer(new QmitkFunctionalityCoordinator);
    m_FunctionalityCoordinator->Start();
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

  m_FunctionalityCoordinator->Stop();
  m_FunctionalityCoordinator = 0;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_common_legacy, QmitkCommonLegacyActivator)
#endif
