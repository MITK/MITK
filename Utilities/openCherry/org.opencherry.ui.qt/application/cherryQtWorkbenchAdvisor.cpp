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

#include "cherryQtWorkbenchAdvisor.h"

#include <cherryPlatform.h>

#include <QApplication>

#include <vector>

namespace cherry
{

void QtWorkbenchAdvisor::Initialize(IWorkbenchConfigurer::Pointer configurer)
{
  WorkbenchAdvisor::Initialize(configurer);

  char** argv;
  int& argc = Platform::GetRawApplicationArgs(argv);

  std::cout << "QtWorkbenchAdvisor::Initialize argv[0] = " << argv[0] << " argv[1] = " << argv[1] << std::endl;

//  const std::vector<std::string> args = Platform::GetApplicationArgs();
//  int* argc = new int;
//  *argc = args.size();
//  char** argv = new char*[*argc];
//
//  int counter = 0;
//  for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it, ++counter)
//  {
//    argv[counter] = const_cast<char*>(it->c_str());
//  }

  new QApplication(argc, argv);

  //delete[] argv;
}

}
