/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPlugin.h"
#include "internal/berryInternalPlatform.h"

namespace berry {

Plugin::Plugin()
  : m_Context(nullptr)
{
}

void Plugin::start(ctkPluginContext* context)
{
  m_Context = context;
}

void Plugin::stop(ctkPluginContext* /*context*/)
{
  m_Context = nullptr;
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
