/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "berryCTKPluginListener_p.h"

#include <ctkPlugin.h>

#include <QSet>

const QString berry::CTKPluginListener::PLUGIN_MANIFEST = "plugin.xml";

namespace berry {

CTKPluginListener::CTKPluginListener(IExtensionPointService::Pointer registry)
  : registry(registry)
{
}

void CTKPluginListener::processPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins)
{
  // sort the plugins according to their dependencies
  const QList<QSharedPointer<ctkPlugin> > sortedPlugins = sortPlugins(plugins);

  foreach (QSharedPointer<ctkPlugin> plugin, sortedPlugins)
  {
    if (isPluginResolved(plugin))
      addPlugin(plugin);
    else
      removePlugin(plugin);
  }
}

QList<QSharedPointer<ctkPlugin> >
CTKPluginListener::sortPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins)
{
  QList<QSharedPointer<ctkPlugin> > sortedPlugins(plugins);
  QSet<QString> installedSymbolicNames;

  QHash<long, QStringList> mapPluginIdToDeps;
  foreach(QSharedPointer<ctkPlugin> plugin, sortedPlugins)
  {
    installedSymbolicNames.insert((plugin->getSymbolicName()));

    QString requirePlugin = plugin->getHeaders()[ctkPluginConstants::REQUIRE_PLUGIN];
    QStringList requiredList = requirePlugin.split(QRegExp("\\s*,\\s*"), QString::SkipEmptyParts);

    QStringList requiredSymbolicNames;
    foreach(QString require, requiredList)
    {
      requiredSymbolicNames.append(require.split(';').front());
    }
    mapPluginIdToDeps[plugin->getPluginId()] = requiredSymbolicNames;
  }

  QStringList stableSymbolicNames;
  for (int i = 0; i < sortedPlugins.size();)
  {
    QStringList currDeps = mapPluginIdToDeps[sortedPlugins.at(i)->getPluginId()];
    bool moved = false;
    foreach(QString currDep, currDeps)
    {
      if (!stableSymbolicNames.contains(currDep) && installedSymbolicNames.contains(currDep))
      {
        sortedPlugins.move(i, sortedPlugins.size()-1);
        moved = true;
        break;
      }
    }
    if (!moved)
    {
      stableSymbolicNames.append(sortedPlugins.at(i)->getSymbolicName());
      ++i;
    }
  }

  return sortedPlugins;
}

void CTKPluginListener::pluginChanged(const ctkPluginEvent& event)
{
  /* Only should listen for RESOLVED and UNRESOLVED events.
   *
   * When a plugin is updated the Framework will publish an UNRESOLVED and
   * then a RESOLVED event which should cause the plugin to be removed
   * and then added back into the registry.
   *
   * When a plugin is uninstalled the Framework should publish an UNRESOLVED
   * event and then an UNINSTALLED event so the plugin will have been removed
   * by the UNRESOLVED event before the UNINSTALLED event is published.
   */
  QSharedPointer<ctkPlugin> plugin = event.getPlugin();
  switch (event.getType())
  {
    case ctkPluginEvent::RESOLVED :
      addPlugin(plugin);
      break;
    case ctkPluginEvent::UNRESOLVED :
      removePlugin(plugin);
      break;
  }
}

bool CTKPluginListener::isPluginResolved(QSharedPointer<ctkPlugin> plugin)
{
  return (plugin->getState() & (ctkPlugin::RESOLVED | ctkPlugin::ACTIVE | ctkPlugin::STARTING | ctkPlugin::STOPPING)) != 0;
}

void CTKPluginListener::removePlugin(QSharedPointer<ctkPlugin> plugin)
{
  // The BlueBerry extension point registry does not support the removal of contributions
  //registry->remove(plugin->getPluginId(), timestamp);
}

QString CTKPluginListener::getExtensionPath(QSharedPointer<ctkPlugin> plugin)
{
  // bail out if system plugin
  if (plugin->getPluginId() == 0)
    return QString();
  // bail out if the plugin does not have a symbolic name
  if (plugin->getSymbolicName().isEmpty())
    return QString();

  return PLUGIN_MANIFEST;
}

void CTKPluginListener::addPlugin(QSharedPointer<ctkPlugin> plugin)
{
  // if the given plugin already exists in the registry then return.
  // note that this does not work for update cases.
  std::string contributor = plugin->getSymbolicName().toStdString();
  if (registry->HasContributionFrom(contributor))
  {
    return;
  }

  QString pluginManifest = getExtensionPath(plugin);
  if (pluginManifest.isEmpty())
    return;

  QByteArray ba = plugin->getResource(pluginManifest);
  if (ba.isEmpty())
    return;

  std::string strContent(ba.data());
  std::stringbuf strBuf(strContent, std::ios_base::in);
  std::istream is(&strBuf);

  registry->AddContribution(is, contributor);
}

}

