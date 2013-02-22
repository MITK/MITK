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
#include <mitkNodePredicateDataType.h>
#include <QmitkNodeDescriptorManager.h>
#include <QtPlugin>

void mitk::org_mitk_gui_qt_simulation_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimulationPreferencePage, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimulationView, context);

  QmitkNodeDescriptorManager* nodeDescriptorManager = QmitkNodeDescriptorManager::GetInstance();

  if (nodeDescriptorManager != NULL)
  {
    mitk::NodePredicateDataType::Pointer isSimulation = mitk::NodePredicateDataType::New("Simulation");
    nodeDescriptorManager->AddDescriptor(new QmitkNodeDescriptor("Simulation", ":/Simulation/icon.png", isSimulation, nodeDescriptorManager));
  }
}

void mitk::org_mitk_gui_qt_simulation_Activator::stop(ctkPluginContext*)
{
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_simulation, mitk::org_mitk_gui_qt_simulation_Activator)
