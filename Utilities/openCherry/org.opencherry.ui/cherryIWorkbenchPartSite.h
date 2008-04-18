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

#ifndef IWORKBENCHPARTSITE_H_
#define IWORKBENCHPARTSITE_H_


#include "org.opencherry.osgi/cherryMacros.h"

namespace cherry {

struct IWorkbenchPart;

struct CHERRY_API IWorkbenchPartSite : public Object
{
  
  cherryClassMacro(IWorkbenchPartSite);
      
  virtual std::string GetId() = 0;
  virtual SmartPointer<IWorkbenchPart> GetPart() = 0;
  virtual std::string GetPluginId() = 0;
  virtual std::string GetRegisteredName() = 0;
  
  // ISelectionProvider GetSelectionProvider() = 0;
  // void SetSelectionProvider(ISelectionProvider provider) = 0;
  
  // void RegisterContextMenu(MenuManager menuManager, ISelectionProvider selectionProvider) = 0;
  // void RegisterContextMenu(QString menuId, MenuManager menuManager, ISelectionProvider selectionProvider) = 0;
  
};

} // namespace cherry


#endif /*IWORKBENCHPARTSITE_H_*/
