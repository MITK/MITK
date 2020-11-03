/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPlatformPropertyTester.h"

#include "berryPlatform.h"
#include <berryObjectString.h>
#include <berryObjectTypeInfo.h>

#include <berryIProduct.h>


namespace berry {

const QString PlatformPropertyTester::PROPERTY_PRODUCT = "product";
const QString PlatformPropertyTester::PROPERTY_IS_PLUGIN_INSTALLED = "isPluginInstalled";
const QString PlatformPropertyTester::PROPERTY_PLUGIN_STATE = "pluginState";


bool PlatformPropertyTester::Test(Object::ConstPointer receiver, const QString &property,
                                  const QList<Object::Pointer> &args, Object::Pointer expectedValue)
{
  if (ObjectTypeInfo::ConstPointer typeInfo = receiver.Cast<const ObjectTypeInfo>())
  {
    if (!(*typeInfo == Reflection::TypeInfo::New<Platform>()))
    {
      return false;
    }

    if (PROPERTY_PRODUCT == property)
    {
      IProduct::Pointer product= Platform::GetProduct();
      if (product.IsNotNull())
      {
        return product->GetId() == expectedValue->ToString();
      }
      return false;
    }
    else if (PROPERTY_IS_PLUGIN_INSTALLED == property && !args.isEmpty() && args[0].Cast<ObjectString>())
    {
      return !Platform::GetPlugin(args[0]->ToString()).isNull();
    }
    else if (PROPERTY_PLUGIN_STATE == property &&
             !args.isEmpty() && args[0].Cast<ObjectString>())
    {
      QSharedPointer<ctkPlugin> p = Platform::GetPlugin(args[0]->ToString());
      if (!p.isNull())
      {
        return PluginState(p->getState(), expectedValue->ToString());
      }
      return false;
    }
  }
  return false;
}

bool PlatformPropertyTester::PluginState(ctkPlugin::State pluginState, const QString& expectedValue)
{
  switch (pluginState)
  {
  case ctkPlugin::UNINSTALLED:
    return expectedValue == QString("UNINSTALLED");
  case ctkPlugin::INSTALLED:
    return expectedValue == QString("INSTALLED");
  case ctkPlugin::RESOLVED:
    return expectedValue == QString("RESOLVED");
  case ctkPlugin::STARTING:
    return expectedValue == QString("STARTING");
  case ctkPlugin::STOPPING:
    return expectedValue == QString("STOPPING");
  case ctkPlugin::ACTIVE:
    return expectedValue == QString("ACTIVE");
  default:
    return false;
  }
  return false;
}

}
