/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_common_legacy, QmitkCommonLegacyActivator)
