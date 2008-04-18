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

#ifndef CHERRYIWORKBENCHWINDOW_H_
#define CHERRYIWORKBENCHWINDOW_H_

#include <org.opencherry.osgi/cherryMacros.h>

namespace cherry {

class IPartService;
class ISelectionService;
class IWorkbenchPage;

struct CHERRY_API IWorkbenchWindow : public Object
{
  
  cherryClassMacro(IWorkbenchWindow);
  
  virtual SmartPointer<IWorkbenchPage> GetActivePage() = 0;
  
  virtual IPartService* GetPartService() = 0;
  
  virtual ISelectionService* GetSelectionService() = 0;
  
  virtual ~IWorkbenchWindow() {}
  
};

}

#endif /*CHERRYIWORKBENCHWINDOW_H_*/
