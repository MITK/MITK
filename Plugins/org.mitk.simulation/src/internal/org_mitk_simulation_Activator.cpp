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
#include <mitkIPropertyFilters.h>
#include <mitkMeshMitkLoader.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPropertyFilter.h>
#include <mitkSimulationObjectFactory.h>
#include <QmitkNodeDescriptorManager.h>
#include <QtPlugin>
#include <sofa/core/ObjectFactory.h>
#include <sofa/helper/system/PluginManager.h>

static void RegisterSofaClasses()
{
  int MeshMitkLoaderClass = sofa::core::RegisterObject("").add<mitk::MeshMitkLoader>();
}

static void LoadSofaPlugins()
{
  berry::IPreferences::Pointer preferences = mitk::GetSimulationPreferences();

  if (preferences.IsNull())
    return;

  QString plugins = preferences->GetByteArray("plugins", "").c_str();

  if (plugins.isEmpty())
    return;

  QStringList pluginList = plugins.split(';', QString::SkipEmptyParts);
  QStringListIterator it(pluginList);

  typedef sofa::helper::system::PluginManager PluginManager;
  PluginManager& pluginManager = PluginManager::getInstance();

  while (it.hasNext())
  {
    std::string plugin = it.next().toStdString();
    std::ostringstream errlog;

    pluginManager.loadPlugin(plugin, &errlog);

    if (errlog.str().empty())
      pluginManager.getPluginMap()[plugin].initExternalModule();
  }
}

static void AddPropertyFilters()
{
  mitk::IPropertyFilters* filters = mitk::org_mitk_simulation_Activator::GetService<mitk::IPropertyFilters>();

  if (filters == NULL)
    return;

  mitk::PropertyFilter filter;

  filter.AddEntry("layer", mitk::PropertyFilter::Blacklist);
  filter.AddEntry("name", mitk::PropertyFilter::Blacklist);
  filter.AddEntry("path", mitk::PropertyFilter::Blacklist);
  filter.AddEntry("selected", mitk::PropertyFilter::Blacklist);
  filter.AddEntry("visible", mitk::PropertyFilter::Blacklist);

  filters->AddFilter(filter, "Simulation");
}

ctkPluginContext* mitk::org_mitk_simulation_Activator::Context = NULL;

void mitk::org_mitk_simulation_Activator::start(ctkPluginContext* context)
{
  Context = context;

  RegisterSimulationObjectFactory();
  RegisterSofaClasses();
  LoadSofaPlugins();
  AddPropertyFilters();

  QmitkNodeDescriptorManager* nodeDescriptorManager = QmitkNodeDescriptorManager::GetInstance();

  if (nodeDescriptorManager != NULL)
  {
    mitk::NodePredicateDataType::Pointer simulationPredicate = mitk::NodePredicateDataType::New("Simulation");
    nodeDescriptorManager->AddDescriptor(new QmitkNodeDescriptor("Simulation", ":/Simulation/SOFAIcon.png", simulationPredicate, nodeDescriptorManager));
  }
}

void mitk::org_mitk_simulation_Activator::stop(ctkPluginContext*)
{
  Context = NULL;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_simulation, mitk::org_mitk_simulation_Activator)
#endif
