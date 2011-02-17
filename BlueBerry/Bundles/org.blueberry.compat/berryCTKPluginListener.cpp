/*=============================================================================

  Library: CTK

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#include "berryCTKPluginListener_p.h"

#include <ctkPlugin.h>

#include <QDebug>

const QString berry::CTKPluginListener::PLUGIN_MANIFEST = "plugin.xml";

namespace berry {

CTKPluginListener::CTKPluginListener(IExtensionPointService::Pointer registry)
  : registry(registry)
{
}

void CTKPluginListener::processPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins)
{
  foreach (QSharedPointer<ctkPlugin> plugin, plugins)
  {
    if (isPluginResolved(plugin))
      addPlugin(plugin);
    else
      removePlugin(plugin);
  }
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
  qDebug() << "**** CTKPluginListener adding plugin:" << plugin->getSymbolicName();
  // if the given plugin already exists in the registry then return.
  // note that this does not work for update cases.
  std::string contributor = plugin->getSymbolicName().toStdString();
  if (registry->HasContributionFrom(contributor))
  {
    qDebug() << "    Contributor already registered:" << plugin->getSymbolicName();
    return;
  }

  QString pluginManifest = getExtensionPath(plugin);
  qDebug() << "    plugin.xml path:" << pluginManifest;
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

