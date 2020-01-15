/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtLogPlugin.h"
#include "berryLogView.h"

namespace berry {

QtLogPlugin* QtLogPlugin::instance = nullptr;

QtLogPlugin::QtLogPlugin()
  : m_LogModel(nullptr)
  , m_Context(nullptr)
{
  this->instance = this;
}

void
QtLogPlugin::start(ctkPluginContext* context)
{
  m_Context = context;

  BERRY_REGISTER_EXTENSION_CLASS(berry::LogView, context)

  m_LogModel = new QtPlatformLogModel();
}

void
QtLogPlugin::stop(ctkPluginContext* /*context*/)
{
  delete m_LogModel;
  m_LogModel = nullptr;
  m_Context = nullptr;
}

QtLogPlugin*
QtLogPlugin::GetInstance()
{
  return instance;
}

QtPlatformLogModel*
QtLogPlugin::GetLogModel() const
{
  return m_LogModel;
}

ctkPluginContext *QtLogPlugin::GetContext() const
{
  return this->m_Context;
}

}
