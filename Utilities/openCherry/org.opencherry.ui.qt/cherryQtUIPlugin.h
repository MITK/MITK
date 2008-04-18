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

#ifndef CHERRYUIPLUGIN_H_
#define CHERRYUIPLUGIN_H_

#include "org.opencherry.osgi/cherryPlugin.h"

#include "internal/cherryQtPlatformLogModel.h"


namespace cherry {

class CHERRY_API QtUIPlugin : public Plugin
{
public:
  
  QtUIPlugin();
  
  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);
  
  static QtUIPlugin* GetInstance();
  
  QtPlatformLogModel* GetLogModel();
  
private:
  static QtUIPlugin* instance;
  QtPlatformLogModel* m_LogModel;
  
};

}

#endif /*CHERRYUIPLUGIN_H_*/
