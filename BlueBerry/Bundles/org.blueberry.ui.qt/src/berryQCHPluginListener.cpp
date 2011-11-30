/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#include "berryQCHPluginListener.h"

#include <ctkPlugin.h>

#include <QDir>
#include <QDateTime>

#include "../berryQtAssistantUtil.h"

namespace berry {

QCHPluginListener::QCHPluginListener(ctkPluginContext* context)
  : delayRegistration(true), context(context)
{
}

void QCHPluginListener::processPlugins()
{
  if (QtAssistantUtil::GetHelpCollectionFile().isEmpty())
  {
    return;
  }

  foreach (QSharedPointer<ctkPlugin> plugin, context->getPlugins())
  {
    if (isPluginResolved(plugin))
      addPlugin(plugin);
    else
      removePlugin(plugin);
  }

  delayRegistration = false;
}

void QCHPluginListener::pluginChanged(const ctkPluginEvent& event)
{
  if (delayRegistration)
  {
    this->processPlugins();
    return;
  }

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

bool QCHPluginListener::isPluginResolved(QSharedPointer<ctkPlugin> plugin)
{
  return (plugin->getState() & (ctkPlugin::RESOLVED | ctkPlugin::ACTIVE | ctkPlugin::STARTING | ctkPlugin::STOPPING)) != 0;
}

void QCHPluginListener::removePlugin(QSharedPointer<ctkPlugin> plugin)
{
  // bail out if system plugin
  if (plugin->getPluginId() == 0) return;

  QFileInfo qchDirInfo = context->getDataFile("qch_files/" + QString::number(plugin->getPluginId()));
  if (qchDirInfo.exists())
  {
    QDir qchDir(qchDirInfo.absoluteFilePath());
    QStringList qchEntries = qchDir.entryList(QStringList("*.qch"));
    QStringList qchFiles;
    foreach(QString qchEntry, qchEntries)
    {
      qchFiles << qchDir.absoluteFilePath(qchEntry);
    }
    // unregister the qch files
    QtAssistantUtil::UnregisterQCHFiles(qchFiles);
    // clean the directory
    foreach(QString qchEntry, qchEntries)
    {
      qchDir.remove(qchEntry);
    }
  }
}

void QCHPluginListener::addPlugin(QSharedPointer<ctkPlugin> plugin)
{
  // bail out if system plugin
  if (plugin->getPluginId() == 0) return;

  QFileInfo qchDirInfo = context->getDataFile("qch_files/" + QString::number(plugin->getPluginId()));
  QUrl location(plugin->getLocation());
  QFileInfo pluginFileInfo(location.toLocalFile());

  if (!qchDirInfo.exists() || qchDirInfo.lastModified() < pluginFileInfo.lastModified())
  {
    removePlugin(plugin);

    if (!qchDirInfo.exists())
    {
      QDir().mkpath(qchDirInfo.absoluteFilePath());
    }

    QStringList localQCHFiles;
    QStringList resourceList = plugin->findResources("/", "*.qch", true);
    foreach(QString resource, resourceList)
    {
      QByteArray content = plugin->getResource(resource);
      QFile localFile(qchDirInfo.absoluteFilePath() + "/" + resource.section('/', -1));
      localFile.open(QIODevice::WriteOnly);
      localFile.write(content);
      localFile.close();
      if (localFile.error() != QFile::NoError)
      {
        BERRY_WARN << "Error writing " << localFile.fileName().toStdString()
                   << ": " << localFile.errorString().toStdString();
      }
      else
      {
        localQCHFiles << localFile.fileName();
      }
    }

    QtAssistantUtil::RegisterQCHFiles(localQCHFiles);
  }

}

}

