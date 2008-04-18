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

#include "org.opencherry.osgi/cherryMacros.h"

#include "dialogs/cherryIDialog.h"
#include "cherryIViewRegistry.h"
#include "cherryIWorkbenchWindow.h"

namespace cherry {

struct CHERRY_API IWorkbench : public Object {
  
  cherryClassMacro(IWorkbench);
  
  static const std::string DIALOG_ID_SHOW_VIEW; // = "org.opencherry.ui.dialogs.showview";
    
  virtual ~IWorkbench() {}
  
  virtual IWorkbenchWindow::Pointer GetActiveWorkbenchWindow() = 0;
  
  virtual IViewRegistry* GetViewRegistry() = 0;
  //virtual IEditorRegistry* GetEditorRegistry() = 0;
  
  
  virtual void ShowPerspective(const std::string& perspectiveId) = 0;
  
  virtual IDialog::Pointer CreateDialog(const std::string& id) = 0;
  
protected:
  
  friend class PlatformUI;
  
  virtual void Run() = 0;
  
};

}

#endif /*CHERRYIWORKBENCH_H_*/
