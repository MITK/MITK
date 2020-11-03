/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
