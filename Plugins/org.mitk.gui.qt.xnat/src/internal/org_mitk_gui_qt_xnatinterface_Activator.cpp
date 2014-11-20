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

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include <QtPlugin>

#include "QmitkXnatEditor.h"
#include "QmitkXnatTreeBrowserView.h"
#include "QmitkXnatConnectionPreferencePage.h"

#include <usGetModuleContext.h>
#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk {

ctkPluginContext* org_mitk_gui_qt_xnatinterface_Activator::m_Context = 0;
us::ModuleContext* org_mitk_gui_qt_xnatinterface_Activator::m_ModuleContext = 0;

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

  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatEditor, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatTreeBrowserView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatConnectionPreferencePage, context)
}

void org_mitk_gui_qt_xnatinterface_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  Q_UNUSED(us::GetModuleContext())
  this->m_Context = 0;
  this->m_ModuleContext = 0;
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_xnatinterface, mitk::org_mitk_gui_qt_xnatinterface_Activator)
#endif
