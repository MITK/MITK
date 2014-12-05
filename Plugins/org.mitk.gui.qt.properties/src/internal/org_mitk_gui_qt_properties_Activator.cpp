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

#include "org_mitk_gui_qt_properties_Activator.h"
#include "QmitkPropertiesPreferencePage.h"
#include "QmitkPropertyTreeView.h"
#include <QtPlugin>

ctkPluginContext* mitk::org_mitk_gui_qt_properties_Activator::m_Context = NULL;

ctkPluginContext* mitk::org_mitk_gui_qt_properties_Activator::GetContext()
{
  return m_Context;
}

void mitk::org_mitk_gui_qt_properties_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPropertiesPreferencePage, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPropertyTreeView, context);
  m_Context = context;
}

void mitk::org_mitk_gui_qt_properties_Activator::stop(ctkPluginContext*)
{
  m_Context = NULL;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_properties, mitk::org_mitk_gui_qt_properties_Activator)
#endif
