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

#ifndef CHERRYIWORKBENCH_H_
#define CHERRYIWORKBENCH_H_

#include "cherryMacros.h"

#include "dialogs/cherryIDialog.h"
#include "cherryIViewRegistry.h"
#include "cherryIEditorRegistry.h"
#include "cherryIWorkbenchWindow.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 */
struct CHERRY_UI IWorkbench : public Object {
  
  cherryClassMacro(IWorkbench);
    
  virtual ~IWorkbench() {}
  
  virtual IWorkbenchWindow::Pointer GetActiveWorkbenchWindow() = 0;
  
  virtual IViewRegistry* GetViewRegistry() = 0;
  virtual IEditorRegistry* GetEditorRegistry() = 0;
  
  
  virtual void ShowPerspective(const std::string& perspectiveId) = 0;
  
};

}

#endif /*CHERRYIWORKBENCH_H_*/
