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

#ifndef CHERRYWORKBENCH_H_
#define CHERRYWORKBENCH_H_

#include <org.opencherry.ui/cherryIViewPart.h>
#include <org.opencherry.ui/cherryIViewPane.h>

#include "cherryIWorkbench.h"
#include "cherryIWorkbenchPartReference.h"
#include <Poco/Exception.h>

namespace cherry {

class ViewRegistry;

class CHERRY_UI Workbench : public IWorkbench
{
  
public:
 
  cherryClassMacro(Workbench);
  
  static const std::string DIALOG_ID_SHOW_VIEW; // = "org.opencherry.ui.dialogs.showview";
  
  Workbench();
  virtual ~Workbench();
  
  IViewRegistry* GetViewRegistry();
  //IEditorRegistry* GetEditorRegistry();
  
  
  void ShowPerspective(const std::string& perspectiveId);
  

  virtual IDialog::Pointer CreateStandardDialog(const std::string& id) = 0;
  
  virtual IViewPart::Pointer CreateErrorViewPart(const std::string& partName, Poco::Exception exc) = 0;
  
  virtual IViewPane::Pointer CreateViewPane() = 0;
  
protected:
  
  friend class PlatformUI;
  
  virtual void Run() = 0;
  
private:
  
  ViewRegistry* m_ViewRegistry;
 
};

} // namespace cherry

#endif /*CHERRYWORKBENCH_H_*/
