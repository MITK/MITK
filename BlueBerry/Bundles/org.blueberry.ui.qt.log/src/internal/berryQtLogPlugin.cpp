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

namespace berry {

QtLogPlugin* QtLogPlugin::instance = 0;

QtLogPlugin::QtLogPlugin()
{
  instance = this;
}

void 
QtLogPlugin::Start(IBundleContext::Pointer /*context*/)
{
  m_LogModel = new QtPlatformLogModel();
}

void 
QtLogPlugin::Stop(IBundleContext::Pointer /*context*/)
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
