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

#ifndef IWORKBENCHPART_H_
#define IWORKBENCHPART_H_

#include <org.opencherry.osgi/cherryDll.h>
#include "org.opencherry.osgi/cherryMacros.h"


namespace cherry {

struct IWorkbenchPartSite;

struct CHERRY_API IWorkbenchPart : public Object
{

  cherryClassMacro(IWorkbenchPart);
  
  //void addPropertyListener(IPropertyListener& listener);
  //void removePropertyListener(IPropertyListener& listener);
  
  virtual void* CreatePartControl(void* parent) = 0;
  virtual SmartPointer<IWorkbenchPartSite> GetSite() = 0;
  virtual const std::string& GetPartName() = 0;
  virtual const std::string& GetContentDescription() = 0;
  virtual void* GetTitleImage() = 0;
  virtual const std::string& GetTitleToolTip() = 0;
  virtual void SetFocus() = 0;
};

}  // namespace cherry

#endif /*IWORKBENCHPART_H_*/
