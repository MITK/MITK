/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_xnatinterface_Activator.h"
#include "QmitkUploadToXNATAction.h"
#include "QmitkXnatTreeBrowserView.h"
#include "QmitkXnatConnectionPreferencePage.h"

#include <usGetModuleContext.h>
#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk {

ctkPluginContext* org_mitk_gui_qt_xnatinterface_Activator::m_Context = nullptr;
us::ModuleContext* org_mitk_gui_qt_xnatinterface_Activator::m_ModuleContext = nullptr;

QmitkXnatSessionManager* org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()
{
  static QmitkXnatSessionManager manager;
  return &manager;
}

ctkPluginContext* org_mitk_gui_qt_xnatinterface_Activator::GetContext()
{
  return m_Context;
}

us::ModuleContext* org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()
{
  return m_ModuleContext;
}

void org_mitk_gui_qt_xnatinterface_Activator::start(ctkPluginContext* context)
{
  this->m_Context = context;
  this->m_ModuleContext = us::GetModuleContext();

  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatTreeBrowserView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatConnectionPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkUploadToXNATAction, context)
}

void org_mitk_gui_qt_xnatinterface_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  Q_UNUSED(us::GetModuleContext())
  this->m_Context = nullptr;
  this->m_ModuleContext = nullptr;
}

}
