/*=========================================================================
 
Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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

Q_EXPORT_PLUGIN2(org_blueberry_ui_qt_log, berry::QtLogPlugin)
