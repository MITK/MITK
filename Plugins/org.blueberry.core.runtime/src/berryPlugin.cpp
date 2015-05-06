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

#include "berryPlugin.h"
#include "internal/berryInternalPlatform.h"

namespace berry {

Plugin::Plugin()
  : m_Context(NULL)
{
}

void Plugin::start(ctkPluginContext* context)
{
  m_Context = context;
}

void Plugin::stop(ctkPluginContext* /*context*/)
{
  m_Context = NULL;
}

QSharedPointer<ctkPlugin> Plugin::GetPlugin() const
{
  return m_Context->getPlugin();
}

ILog* Plugin::GetLog() const
{
  return InternalPlatform::GetInstance()->GetLog(m_Context->getPlugin());
}

QString Plugin::GetStateLocation() const
{
  return InternalPlatform::GetInstance()->GetStateLocation(GetPlugin()).absolutePath();
}

}
