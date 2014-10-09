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

#include <QtPlugin>

namespace berry {

QtLogPlugin* QtLogPlugin::instance = 0;

QtLogPlugin::QtLogPlugin()
{
  instance = this;
}

void
QtLogPlugin::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(berry::LogView, context)

  m_LogModel = new QtPlatformLogModel();
}

void
QtLogPlugin::stop(ctkPluginContext* /*context*/)
{
  delete m_LogModel;
}

QtLogPlugin*
QtLogPlugin::GetInstance()
{
  return instance;
}

QtPlatformLogModel*
QtLogPlugin::GetLogModel()
{
  return m_LogModel;
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_ui_qt_log, berry::QtLogPlugin)
#endif
