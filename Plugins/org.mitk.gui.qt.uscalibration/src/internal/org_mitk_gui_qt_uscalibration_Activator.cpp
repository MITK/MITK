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

#include "org_mitk_gui_qt_uscalibration_Activator.h"

#include <QtPlugin>

#include "UltrasoundCalibration.h"

namespace mitk {
ctkPluginContext* org_mitk_gui_qt_uscalibration_Activator::m_Context = 0;

void org_mitk_gui_qt_uscalibration_Activator::start(ctkPluginContext* context)
{
  m_Context = context;

  BERRY_REGISTER_EXTENSION_CLASS(UltrasoundCalibration, context)
}

void org_mitk_gui_qt_uscalibration_Activator::stop(ctkPluginContext* context)
{
  m_Context = 0;

  Q_UNUSED(context)
}

ctkPluginContext *org_mitk_gui_qt_uscalibration_Activator::GetContext()
{
  return m_Context;
}
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_uscalibration, mitk::org_mitk_gui_qt_uscalibration_Activator)
