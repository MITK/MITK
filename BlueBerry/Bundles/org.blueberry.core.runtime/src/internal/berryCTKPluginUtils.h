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

#ifndef BERRYCTKPLUGINUTILS_H
#define BERRYCTKPLUGINUTILS_H

#include <ctkServiceTracker.h>

#include <QSharedPointer>

class ctkPlugin;

namespace berry {

/**
 * The class contains a set of helper methods for the runtime content plugin.
 * The closeServices() method should be called before the plugin is stopped.
 */
class CTKPluginUtils
{

private:

  //QScopedPointer<ctkServiceTracker> debugTracker;
  //QScopedPointer<ctkServiceTracker> configurationLocationTracker ;

  /**
   * Private constructor to block instance creation.
   */
  CTKPluginUtils();

  void InitServices();

public:

  static const QString PROP_CONFIG_AREA; // = "blueberry.configuration.area";
  static const QString PROP_INSTANCE_AREA; // = "blueberry.instance.area";

  static CTKPluginUtils* GetDefault();

  void CloseServices();

  bool GetBoolDebugOption(const QString& option, bool defaultValue) const;

  QSharedPointer<ctkPlugin> GetPlugin(const QString& pluginName);

  //Location GetConfigurationLocation() const;
};

}

#endif // BERRYCTKPLUGINUTILS_H
