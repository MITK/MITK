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

#include "QmitkWorkbenchWindowAdvisor.h"
#include "QmitkActionBarAdvisor.h"

QmitkWorkbenchWindowAdvisor::QmitkWorkbenchWindowAdvisor(cherry::IWorkbenchWindowConfigurer::Pointer configurer)
 : cherry::WorkbenchWindowAdvisor(configurer)
{
  
}

cherry::ActionBarAdvisor::Pointer 
QmitkWorkbenchWindowAdvisor::CreateActionBarAdvisor(
    cherry::IActionBarConfigurer::Pointer configurer)
{
  cherry::ActionBarAdvisor::Pointer actionBarAdvisor = new QmitkActionBarAdvisor(configurer);
  return actionBarAdvisor;
}
