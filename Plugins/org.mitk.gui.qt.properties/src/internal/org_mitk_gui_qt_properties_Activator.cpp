/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_properties_Activator.h"
#include "QmitkPropertyTreeView.h"

ctkPluginContext* mitk::org_mitk_gui_qt_properties_Activator::m_Context = nullptr;

ctkPluginContext* mitk::org_mitk_gui_qt_properties_Activator::GetContext()
{
  return m_Context;
}

void mitk::org_mitk_gui_qt_properties_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPropertyTreeView, context);
  m_Context = context;
}

void mitk::org_mitk_gui_qt_properties_Activator::stop(ctkPluginContext*)
{
  m_Context = nullptr;
}
