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

#include "QmitkCommonActivator.h"

#include <berryPlatformUI.h>
#include <mitkLogMacros.h>

ctkPluginContext* QmitkCommonActivator::m_Context = 0;

ctkPluginContext* QmitkCommonActivator::GetContext()
{
  return m_Context;
}

void
QmitkCommonActivator::start(ctkPluginContext* context)
{
  this->m_Context = context;

  if(berry::PlatformUI::IsWorkbenchRunning())
  {
    m_ViewCoordinator = QmitkViewCoordinator::Pointer(new QmitkViewCoordinator);
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
  m_ViewCoordinator = 0;

  this->m_Context = 0;
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_common, QmitkCommonActivator)
