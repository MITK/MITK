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

#ifndef CHERRYIPARTPANE_H_
#define CHERRYIPARTPANE_H_

#include <org.opencherry.osgi/cherryMacros.h>

namespace cherry {

/**
 * The container for workbench parts.
 */
struct CHERRY_API IPartPane : public Object {
  
  cherryClassMacro(IPartPane)
  
  virtual ~IPartPane() {}
  
  /**
   * Returns the GUI-dependent container control
   * for the part widgets. This is passed to
   * IWorkbenchPart::CreatePartControl(void*)
   */
  virtual void* GetControl() = 0;
  
  virtual bool GetVisible() = 0;
  virtual void SetVisible(bool visible) = 0;
  
};

}

#endif /*CHERRYIPARTPANE_H_*/
