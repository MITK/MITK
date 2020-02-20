/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_ultrasound_Activator.h"
#include "QmitkUltrasoundSupport.h"

namespace mitk {

ctkPluginContext* org_mitk_gui_qt_ultrasound_Activator::m_Context = nullptr;

void org_mitk_gui_qt_ultrasound_Activator::start(ctkPluginContext* context)
{
  m_Context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkUltrasoundSupport, context)
}

void org_mitk_gui_qt_ultrasound_Activator::stop(ctkPluginContext* context)
{
  m_Context = nullptr;

  Q_UNUSED(context)
}

ctkPluginContext *org_mitk_gui_qt_ultrasound_Activator::GetContext()
{
  return m_Context;
}

}
