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


#ifndef BERRYCTKPLUGINLISTENER_P_H
#define BERRYCTKPLUGINLISTENER_P_H

#include <QObject>
#include <QSharedPointer>

#include <ctkPluginEvent.h>

#include <berryIExtensionPointService.h>

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

  QList<QSharedPointer<ctkPlugin> > sortPlugins(const QList<QSharedPointer<ctkPlugin> >& plugins);

};

}

#endif // BERRYCTKPLUGINLISTENER_P_H
