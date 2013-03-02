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

#include "org_mitk_simulation_Activator.h"
#include <mitkGetSimulationPreferences.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSimulationObjectFactory.h>
#include <QmitkNodeDescriptorManager.h>
#include <QtPlugin>
#include <sofa/helper/system/PluginManager.h>

static void InitSOFAPlugins()
{
  berry::IPreferences::Pointer preferences = mitk::GetSimulationPreferences();

  if (preferences.IsNull())
    return;

  QString pluginPaths = preferences->GetByteArray("plugin paths", "").c_str();

  if (pluginPaths.isEmpty())
    return;

  QStringList pluginPathList = pluginPaths.split(';', QString::SkipEmptyParts);
  QStringListIterator it(pluginPathList);

  typedef sofa::helper::system::PluginManager PluginManager;
  PluginManager& pluginManager = PluginManager::getInstance();

  while (it.hasNext())
  {
    std::string path = it.next().toStdString();
    std::ostringstream errlog;

    pluginManager.loadPlugin(path, &errlog);

    if (errlog.str().empty())
      pluginManager.getPluginMap()[path].initExternalModule();
  }
}

void mitk::org_mitk_simulation_Activator::start(ctkPluginContext*)
{
  RegisterSimulationObjectFactory();
  InitSOFAPlugins();

  QmitkNodeDescriptorManager* nodeDescriptorManager = QmitkNodeDescriptorManager::GetInstance();

  if (nodeDescriptorManager != NULL)
  {
    mitk::NodePredicateDataType::Pointer isSimulation = mitk::NodePredicateDataType::New("Simulation");
    nodeDescriptorManager->AddDescriptor(new QmitkNodeDescriptor("Simulation", ":/Simulation/simulation.png", isSimulation, nodeDescriptorManager));

    mitk::NodePredicateDataType::Pointer isSimulationTemplate = mitk::NodePredicateDataType::New("SimulationTemplate");
    nodeDescriptorManager->AddDescriptor(new QmitkNodeDescriptor("SimulationTemplate", ":/Simulation/simulationTemplate.png", isSimulationTemplate, nodeDescriptorManager));
  }
}

void mitk::org_mitk_simulation_Activator::stop(ctkPluginContext* context)
{
}

Q_EXPORT_PLUGIN2(org_mitk_simulation, mitk::org_mitk_simulation_Activator)
