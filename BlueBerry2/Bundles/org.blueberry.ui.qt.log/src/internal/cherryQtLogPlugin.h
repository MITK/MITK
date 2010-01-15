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

#ifndef CHERRYLOGPLUGIN_H_
#define CHERRYLOGPLUGIN_H_

#include <cherryPlugin.h>

#include "cherryQtPlatformLogModel.h"


namespace cherry {

class QtLogPlugin : public Plugin
{
public:

  QtLogPlugin();

  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);

  static QtLogPlugin* GetInstance();

  QtPlatformLogModel* GetLogModel();

private:
  static QtLogPlugin* instance;
  QtPlatformLogModel* m_LogModel;

};

}

#endif /*CHERRYLOGPLUGIN_H_*/
