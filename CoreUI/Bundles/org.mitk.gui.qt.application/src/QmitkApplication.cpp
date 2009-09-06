/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkApplication.h"

#include <cherryPlatformUI.h>

#include "internal/QmitkWorkbenchAdvisor.h"

int QmitkApplication::Start()
{
  cherry::Display* display = cherry::PlatformUI::CreateDisplay();

  int code = cherry::PlatformUI::CreateAndRunWorkbench(display, new QmitkWorkbenchAdvisor());
  
  // exit the application with an appropriate return code
  return code == cherry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void QmitkApplication::Stop()
{
  
}
