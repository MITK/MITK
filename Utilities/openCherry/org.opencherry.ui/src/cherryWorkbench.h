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

#include "cherryIViewPart.h"
#include "cherryPartPane.h"

#include "cherryIWorkbench.h"
#include "cherryIWorkbenchPartReference.h"
#include "cherryIEditorAreaHelper.h"

#include <Poco/Exception.h>

namespace cherry {

class ViewRegistry;
class EditorRegistry;

/**
 * \ingroup org_opencherry_ui
 * 
 */
class CHERRY_UI Workbench : public IWorkbench
{
  
public:
 
  cherryClassMacro(Workbench);
  
  static const std::string DIALOG_ID_SHOW_VIEW; // = "org.opencherry.ui.dialogs.showview";
  
  Workbench();
  virtual ~Workbench();
  
  IViewRegistry* GetViewRegistry();
  IEditorRegistry* GetEditorRegistry();
  
  
  void ShowPerspective(const std::string& perspectiveId);
  
  virtual IEditorAreaHelper* CreateEditorPresentation() = 0;

  virtual IDialog::Pointer CreateStandardDialog(const std::string& id) = 0;
  
  virtual IViewPart::Pointer CreateErrorViewPart(const std::string& partName = "", const std::string& msg = "") = 0;
  virtual IEditorPart::Pointer CreateErrorEditorPart(const std::string& partName = "", const std::string& msg = "") = 0;
  
  virtual PartPane::Pointer CreateViewPane(IWorkbenchPartReference::Pointer partReference,
      WorkbenchPage::Pointer workbenchPage) = 0;
  virtual PartPane::Pointer CreateEditorPane() = 0;
  
  virtual void* CreateWorkbenchPageControl() = 0;
  
  // Hack for DummyWorkbenchPage
  virtual void AddViewPane(PartPane::Pointer pane) = 0;
  
protected:
  
  friend class PlatformUI;
  
  virtual void Run() = 0;
  
 
};

} // namespace cherry

#endif /*CHERRYWORKBENCH_H_*/
