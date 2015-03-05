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

#ifndef BERRYCTKPLUGINACTIVATOR_H
#define BERRYCTKPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

#include <QObject>

#include <berrySmartPointer.h>


namespace berry {

//class CTKPluginListener;
class PreferencesService;
struct IExtensionRegistry;

class org_blueberry_core_runtime_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  Q_PLUGIN_METADATA(IID "org_blueberry_core_runtime")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  org_blueberry_core_runtime_Activator();
  ~org_blueberry_core_runtime_Activator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* getPluginContext();

  /**
   * Returns the plug-in id of the plug-in that contains the provided symbol, or
   * a null QString if the plug-in could not be determined.
   */
  static QString getPluginId(void* symbol);

private:

  void startRegistry();
  void stopRegistry();

  static ctkPluginContext* context;

  //QScopedPointer<CTKPluginListener> pluginListener;

  QScopedPointer<PreferencesService> preferencesService;
  ctkServiceRegistration prefServiceReg;

  QScopedPointer<IExtensionRegistry> defaultRegistry;
  ctkServiceRegistration registryServiceReg;

  QScopedPointer<QObject> userRegistryKey;
  QScopedPointer<QObject> masterRegistryKey;

};

typedef org_blueberry_core_runtime_Activator CTKPluginActivator;

}

#endif // BERRYCTKPLUGINACTIVATOR_H
