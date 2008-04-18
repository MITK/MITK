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

#include "cherryQtWorkbenchApplication.h"

#include "org.opencherry.osgi/cherryPlatform.h"
#include "org.opencherry.ui/cherryPlatformUI.h"

#include <QtGui/QApplication>

#include <iostream>

namespace cherry {

int
QtWorkbenchApplication::Run()
{
  //std::cout << "Running sample application\n";
  int argc;
  char** argv;
  Platform::GetRawApplicationArgs(argc, argv);
  std::cout << "argc = " << argc << ", argv = " << argv << std::endl;
  std::cout << "argv[0] = " << argv[0] << std::endl;
  QApplication app(argc, argv);
  PlatformUI::CreateAndRunWorkbench();
  return app.exec();
  
}

}
