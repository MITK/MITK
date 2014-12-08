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

#include "org_mitk_gui_qt_simulation_Activator.h"
#include "QmitkSimulationPreferencePage.h"
#include "QmitkSimulationView.h"
#include <QtPlugin>

ctkPluginContext* mitk::org_mitk_gui_qt_simulation_Activator::Context = NULL;

ctkPluginContext* mitk::org_mitk_gui_qt_simulation_Activator::GetContext()
{
  return Context;
}

void mitk::org_mitk_gui_qt_simulation_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimulationPreferencePage, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimulationView, context);
  Context = context;
}

void mitk::org_mitk_gui_qt_simulation_Activator::stop(ctkPluginContext*)
{
  Context = NULL;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_simulation, mitk::org_mitk_gui_qt_simulation_Activator)
#endif
