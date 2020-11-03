/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  Q_INTERFACES(ctkEventHandler)

public:

  void handleEvent(const ctkEvent& event) override;
};

class HelpPluginActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt_help")
  Q_INTERFACES(ctkPluginActivator)

public:

  HelpPluginActivator();
  ~HelpPluginActivator() override;

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  static HelpPluginActivator* getInstance();

  static void linkActivated(IWorkbenchPage::Pointer page, const QUrl &link);

  QHelpEngineWrapper& getQHelpEngine();

private:

  Q_DISABLE_COPY(HelpPluginActivator)

  static HelpPluginActivator* instance;

  QScopedPointer<QHelpEngineWrapper, QScopedPointerDeleteLater > helpEngine;
  QScopedPointer<QHelpEngineConfiguration> helpEngineConfiguration;
  QScopedPointer<HelpContextHandler> helpContextHandler;

  QCHPluginListener* pluginListener;
  QScopedPointer<IWindowListener> wndListener;
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
