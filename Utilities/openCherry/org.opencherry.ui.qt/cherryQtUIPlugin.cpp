/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryQtUIPlugin.h"

namespace cherry {

QtUIPlugin* QtUIPlugin::instance = 0;

QtUIPlugin::QtUIPlugin()
{
  instance = this;
}

void 
QtUIPlugin::Start(IBundleContext::Pointer /*context*/)
{
  m_LogModel = new QtPlatformLogModel();
}

void 
QtUIPlugin::Stop(IBundleContext::Pointer /*context*/)
{
  delete m_LogModel;
}
  
QtUIPlugin* 
QtUIPlugin::GetInstance()
{
  return instance;
}

QtPlatformLogModel* 
QtUIPlugin::GetLogModel()
{
  return m_LogModel;
}

}
