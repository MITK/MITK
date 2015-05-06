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

#ifndef BERRYPLATFORMPROPERTYTESTER_H
#define BERRYPLATFORMPROPERTYTESTER_H

#include <berryPropertyTester.h>

#include <ctkPlugin.h>

namespace berry {

/**
 * A property tester for testing platform properties. Can test whether or
 * not a given plugin is installed in the running environment, as well as
 * the id of the currently active product.
 * <p>
 * For example:<br />
 * &lt;test property="org.blueberry.core.runtime.product" value="org.mitk.qt.Workbench"/&gt; <br />
 * &lt;test property="org.blueberry.core.runtime.isPluginInstalled" args="org.blueberry.core.expressions"/&gt; <br />
 * &lt;test property="org.blueberry.core.runtime.pluginState" args="org.blueberry.core.expressions" value="ACTIVE"/&gt;
 * <p>
 */
class PlatformPropertyTester : public PropertyTester
{
  Q_OBJECT
private:

  static const QString PROPERTY_PRODUCT; // = "product";
  static const QString PROPERTY_IS_PLUGIN_INSTALLED; // = "isPluginInstalled";
  static const QString PROPERTY_PLUGIN_STATE; // = "pluginState";

public:

  bool Test(Object::ConstPointer receiver, const QString& property,
            const QList<Object::Pointer>& args, Object::Pointer expectedValue);

private:

  bool PluginState(ctkPlugin::State pluginState, const QString &expectedValue);

};

}

#endif // BERRYPLATFORMPROPERTYTESTER_H
