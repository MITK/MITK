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


#ifndef BERRYCTKPLUGINLISTENER_H
#define BERRYCTKPLUGINLISTENER_H

#include <QObject>
#include <QSharedPointer>
#include <QMutex>

#include <ctkPluginEvent.h>

namespace berry {

class ExtensionRegistry;
class RegistryStrategy;

/**
 * A listener for CTK plugin events. When plugins come and go we look to see
 * if there are any extensions or extension points and update the legacy BlueBerry registry accordingly.
 * Using a Synchronous listener here is important. If the
 * plugin activator code tries to access the registry to get its extension
 * points, we need to ensure that they are in the registry before the
 * plugin start is called. By listening sync we are able to ensure that
 * happens.
 */
class CTKPluginListener : public QObject {

  Q_OBJECT

private:

  static const QString PLUGIN_MANIFEST; // = "plugin.xml"

  ExtensionRegistry* registry;
  RegistryStrategy* strategy;
  QObject* token;

  QList<long> currentStateStamp;
  QMutex mutex;

public:

  CTKPluginListener(ExtensionRegistry* registry, QObject* key, RegistryStrategy* strategy);

  void ProcessPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins);

public slots:

  void PluginChanged(const ctkPluginEvent& event);

private:

  bool IsPluginResolved(QSharedPointer<ctkPlugin> plugin);

  void RemovePlugin(QSharedPointer<ctkPlugin> plugin);

  static QString GetExtensionPath(QSharedPointer<ctkPlugin> plugin);

  void AddPlugin(QSharedPointer<ctkPlugin> plugin);

};

}

#endif // BERRYCTKPLUGINLISTENER_H
