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
