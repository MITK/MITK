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

#include "cherryWorkbenchWindow.h"

#include "cherryIWorkbenchPage.h"

namespace cherry
{

SmartPointer<IWorkbenchPage> WorkbenchWindow::GetActivePage()
{
  // TODO: return the active WorkbenchPage
  return SmartPointer<IWorkbenchPage>();
}

IPartService* WorkbenchWindow::GetPartService()
{
  return 0;
}

ISelectionService* WorkbenchWindow::GetSelectionService()
{
  return 0;
}

}
