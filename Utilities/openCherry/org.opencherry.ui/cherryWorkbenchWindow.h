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

#ifndef CHERRYWORKBENCHWINDOW_H_
#define CHERRYWORKBENCHWINDOW_H_

#include "cherryIWorkbenchWindow.h"

namespace cherry {

class CHERRY_API WorkbenchWindow : public IWorkbenchWindow
{
public:
  cherryClassMacro(WorkbenchWindow);
  
  SmartPointer<IWorkbenchPage> GetActivePage();
    
  IPartService* GetPartService();
    
  ISelectionService* GetSelectionService();
  
};

}

#endif /*CHERRYWORKBENCHWINDOW_H_*/
