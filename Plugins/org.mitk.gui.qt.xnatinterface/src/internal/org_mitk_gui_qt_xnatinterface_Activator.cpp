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
#include "QmitkXnatSimpleSearchView.h"
#include "QmitkXnatTreeBrowserView.h"
#include "QmitkXnatConnectionPreferencePage.h"

namespace mitk {

ctkPluginContext* org_mitk_gui_qt_xnatinterface_Activator::m_Context = 0;

QmitkXnatConnectionManager* org_mitk_gui_qt_xnatinterface_Activator::GetXnatConnectionManager()
{
  static QmitkXnatConnectionManager manager;
  return &manager;
}

ctkPluginContext* org_mitk_gui_qt_xnatinterface_Activator::GetContext()
{
  return m_Context;
}

void org_mitk_gui_qt_xnatinterface_Activator::start(ctkPluginContext* context)
{
  this->m_Context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatEditor, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatSimpleSearchView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatTreeBrowserView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatConnectionPreferencePage, context)
}

void org_mitk_gui_qt_xnatinterface_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_xnatinterface, mitk::org_mitk_gui_qt_xnatinterface_Activator)
