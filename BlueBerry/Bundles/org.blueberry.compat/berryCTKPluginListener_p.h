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


#ifndef BERRYCTKPLUGINLISTENER_P_H
#define BERRYCTKPLUGINLISTENER_P_H

#include <QObject>
#include <QSharedPointer>

#include <ctkPluginEvent.h>

#include <src/service/berryIExtensionPointService.h>

class ctkPlugin;

namespace berry {

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

  IExtensionPointService::Pointer registry;

public:

  CTKPluginListener(IExtensionPointService::Pointer registry);

  void processPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins);

public slots:

  void pluginChanged(const ctkPluginEvent& event);

private:

  bool isPluginResolved(QSharedPointer<ctkPlugin> plugin);

  void removePlugin(QSharedPointer<ctkPlugin> plugin);

  static QString getExtensionPath(QSharedPointer<ctkPlugin> plugin);

  void addPlugin(QSharedPointer<ctkPlugin> plugin);

};

}

#endif // BERRYCTKPLUGINLISTENER_P_H
