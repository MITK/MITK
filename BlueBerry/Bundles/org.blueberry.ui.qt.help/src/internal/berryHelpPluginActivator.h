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

#ifndef BERRYLOGPLUGIN_H_
#define BERRYLOGPLUGIN_H_

#include <ctkPluginActivator.h>

#include <service/event/ctkEvent.h>
#include <service/event/ctkEventHandler.h>

#include <QScopedPointer>
#include <QMutex>

#include <berryIWorkbenchPage.h>
#include <berryIWindowListener.h>

class QHelpEngine;

namespace berry {

class QHelpEngineConfiguration;
class QHelpEngineWrapper;
class QCHPluginListener;

class HelpContextHandler : public QObject, public ctkEventHandler
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt_help")
#endif
  Q_INTERFACES(ctkEventHandler)

public:

  void handleEvent(const ctkEvent& event);
};

class HelpPluginActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  HelpPluginActivator();
  ~HelpPluginActivator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static HelpPluginActivator* getInstance();

  static void linkActivated(IWorkbenchPage::Pointer page, const QUrl &link);

  QHelpEngineWrapper& getQHelpEngine();

private:

  Q_DISABLE_COPY(HelpPluginActivator)

  static HelpPluginActivator* instance;

  QScopedPointer<QHelpEngineWrapper> helpEngine;
  QScopedPointer<QHelpEngineConfiguration> helpEngineConfiguration;
  QScopedPointer<HelpContextHandler> helpContextHandler;

  QCHPluginListener* pluginListener;
  IWindowListener::Pointer wndListener;
};

/**
 * A listener for CTK plugin events. When plugins come and go we look to see
 * if there are any qch files and update the QHelpEngine accordingly.
 */
class QCHPluginListener : public QObject {

  Q_OBJECT

public:

  QCHPluginListener(ctkPluginContext* context, QHelpEngine* helpEngine);

  void processPlugins();

public Q_SLOTS:

  void pluginChanged(const ctkPluginEvent& event);

private:

  void processPlugins_unlocked();

  bool isPluginResolved(QSharedPointer<ctkPlugin> plugin);

  void removePlugin(QSharedPointer<ctkPlugin> plugin);
  void addPlugin(QSharedPointer<ctkPlugin> plugin);

  QMutex mutex;
  bool delayRegistration;
  ctkPluginContext* context;
  QHelpEngine* helpEngine;
};

}

#endif /*BERRYLOGPLUGIN_H_*/
