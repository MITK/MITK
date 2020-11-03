/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryCTKPluginListener.h"
#include "berryExtensionRegistry.h"
#include "berryRegistryStrategy.h"
#include "berryIContributor.h"
#include "berryContributorFactory.h"

#include <ctkPlugin.h>

#include <QSet>
#include <QStringList>
#include <QBuffer>

const QString berry::CTKPluginListener::PLUGIN_MANIFEST = "plugin.xml";

namespace berry {

CTKPluginListener::CTKPluginListener(ExtensionRegistry* registry, QObject* key, RegistryStrategy* strategy)
  : registry(registry), strategy(strategy), token(key)
{
  currentStateStamp << 0;
}

void CTKPluginListener::ProcessPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins)
{
  // sort the plugins according to their dependencies
  //const QList<QSharedPointer<ctkPlugin> > sortedPlugins = sortPlugins(plugins);

  foreach (QSharedPointer<ctkPlugin> plugin, plugins)
  {
    if (IsPluginResolved(plugin))
      AddPlugin(plugin);
    else
      RemovePlugin(plugin);
  }
}

void CTKPluginListener::PluginChanged(const ctkPluginEvent& event)
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
  {
    {
      QMutexLocker l(&mutex);
      long newStateStamp = registry->ComputeState();
      if (currentStateStamp[0] != newStateStamp)
      {
        // new state stamp
        currentStateStamp[0] = newStateStamp;
      }
    }
    AddPlugin(plugin);
    break;
  }
  case ctkPluginEvent::UNRESOLVED :
      RemovePlugin(plugin);
      break;
  default: break;
  }
}

bool CTKPluginListener::IsPluginResolved(QSharedPointer<ctkPlugin> plugin)
{
  return (plugin->getState() & (ctkPlugin::RESOLVED | ctkPlugin::ACTIVE | ctkPlugin::STARTING | ctkPlugin::STOPPING)) != 0;
}

void CTKPluginListener::RemovePlugin(QSharedPointer<ctkPlugin> plugin)
{
  long timestamp = 0;
  if (strategy->CheckContributionsTimestamp())
  {
    QString pluginManifest = GetExtensionPath(plugin);
    if (!pluginManifest.isEmpty())
    {
      timestamp = strategy->GetExtendedTimestamp(plugin, pluginManifest);
    }
  }
  registry->Remove(QString::number(plugin->getPluginId()), timestamp);
}

QString CTKPluginListener::GetExtensionPath(QSharedPointer<ctkPlugin> plugin)
{
  // bail out if system plugin
  if (plugin->getPluginId() == 0)
    return QString();
  // bail out if the plugin does not have a symbolic name
  if (plugin->getSymbolicName().isEmpty())
    return QString();

  return PLUGIN_MANIFEST;
}

void CTKPluginListener::AddPlugin(QSharedPointer<ctkPlugin> plugin)
{
  // if the given plugin already exists in the registry then return.
  // note that this does not work for update cases.
  IContributor::Pointer contributor = ContributorFactory::CreateContributor(plugin);
  if (registry->HasContributor(contributor))
  {
    return;
  }

  QString pluginManifest = GetExtensionPath(plugin);
  if (pluginManifest.isEmpty())
    return;

  QByteArray ba = plugin->getResource(pluginManifest);
  if (ba.isEmpty())
    return;

  long timestamp = 0;
  if (strategy->CheckContributionsTimestamp())
  {
    timestamp = strategy->GetExtendedTimestamp(plugin, pluginManifest);
  }

  QBuffer buffer(&ba);
  registry->AddContribution(&buffer, contributor, true, pluginManifest, nullptr, token, timestamp);
}

}

