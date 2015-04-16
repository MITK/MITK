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


#include "berryExpressionPlugin.h"

#include "berryExpressions.h"
#include "berryTypeExtensionManager.h"
#include "berryPlatformPropertyTester.h"

#include "berryPlatform.h"

#include <QtPlugin>

namespace berry {

QObject* ExpressionPlugin::m_PluginListener = NULL;
ExpressionPlugin* ExpressionPlugin::m_Default = NULL;

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
  if (m_PluginListener != NULL)
  {
    context->disconnectPluginListener(m_PluginListener);
  }
  m_PluginListener = NULL;
  Plugin::stop(context);
}

ctkPluginContext* ExpressionPlugin::GetPluginContext() const
{
  return m_Context;
}

}

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  Q_EXPORT_PLUGIN2(org_blueberry_core_expressions, berry::ExpressionPlugin)
#endif
