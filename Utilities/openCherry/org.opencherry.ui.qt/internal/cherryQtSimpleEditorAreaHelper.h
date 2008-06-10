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

#ifndef CHERRYQTEDITORAREAHELPER_H_
#define CHERRYQTEDITORAREAHELPER_H_

#include <org.opencherry.ui/src/cherryIEditorAreaHelper.h>
#include <org.opencherry.ui/src/cherryPartPane.h>

#include <list>
#include <QTabWidget>

namespace cherry
{

class QtSimpleEditorAreaHelper : public IEditorAreaHelper
{

private:

  //EditorSashContainer editorArea;
  QTabWidget* editorArea;
  
  std::list<PartPane::Pointer> editorList;

public:

  QtSimpleEditorAreaHelper(IWorkbenchPage::Pointer page);
  
  QTabWidget* GetControl();

  void CloseEditor(IEditorReference::Pointer ref);

  void CloseEditor(IEditorPart::Pointer part);

  LayoutPart::Pointer GetLayoutPart();

  IEditorReference::Pointer GetVisibleEditor();

  void AddEditor(IEditorReference::Pointer ref);

  bool SetVisibleEditor(IEditorReference::Pointer ref, bool setFocus);

  std::vector<IEditorReference::Pointer> GetEditors();

private:

  void CloseEditor(PartPane::Pointer pane);

  void AddToLayout(PartPane::Pointer pane);

};

}

#endif /*CHERRYQTEDITORAREAHELPER_H_*/
