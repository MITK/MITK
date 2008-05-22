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

#ifndef CHERRYPLATFORMUI_H_
#define CHERRYPLATFORMUI_H_

#include "cherryUiDll.h"
#include "cherryWorkbench.h"

namespace cherry {

class CHERRY_UI PlatformUI
{
public:
  
  static const std::string PLUGIN_ID;
  static const std::string XP_WORKBENCH;
  static const std::string XP_VIEWS;
  
  static int CreateAndRunWorkbench();
  
  static IWorkbench::Pointer GetWorkbench();
  
  static bool IsWorkbenchRunning();
  
private:
  static Workbench::Pointer m_Workbench;
};

}

#endif /*CHERRYPLATFORMUI_H_*/
