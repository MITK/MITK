/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExpressionPlugin.h"
#include "berryExpressions.h"
#include "berryTypeExtensionManager.h"
#include "berryPlatformPropertyTester.h"
#include "berryPlatform.h"

namespace berry {

QObject* ExpressionPlugin::m_PluginListener = nullptr;
ExpressionPlugin* ExpressionPlugin::m_Default = nullptr;

ExpressionPlugin::ExpressionPlugin()
{
  m_Default = this;
}

ExpressionPlugin* ExpressionPlugin::GetDefault()
{
  return m_Default;
}

QString ExpressionPlugin::GetPluginId()
{
  return "org.blueberry.core.expressions";
}

void ExpressionPlugin::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(PlatformPropertyTester, context)

  Expressions::TRACING = Platform::GetDebugOption("org.blueberry.core.expressions/tracePropertyResolving").toBool();
  TypeExtensionManager::DEBUG = Platform::GetDebugOption("org.blueberry.core.expressions/debug/TypeExtensionManager").toBool();
}

void ExpressionPlugin::stop(ctkPluginContext* context)
{
  if (m_PluginListener != nullptr)
  {
    context->disconnectPluginListener(m_PluginListener);
  }
  m_PluginListener = nullptr;
  Plugin::stop(context);
}

ctkPluginContext* ExpressionPlugin::GetPluginContext() const
{
  return m_Context;
}

}
