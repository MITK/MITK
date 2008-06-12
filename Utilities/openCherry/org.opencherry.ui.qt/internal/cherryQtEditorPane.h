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

#ifndef CHERRYQTEDITORPANE_H_
#define CHERRYQTEDITORPANE_H_

#include <cherryPartPane.h>

#include <QWidget>

namespace cherry
{

class QtEditorPane : public PartPane
{
  
public:
  
  cherryClassMacro(QtEditorPane);
  
  QtEditorPane(IWorkbenchPartReference::Pointer partReference,
               WorkbenchPage::Pointer workbenchPage);
  
  virtual ~QtEditorPane();
  
  void CreateControl(void* parent);
  void SetControlEnabled(bool enabled);
  void CreateTitleBar();
  void DoHide();
  Rectangle GetBounds();
  void MoveAbove(void* refControl);
  void ShowFocus(bool inFocus);
  bool IsCloseable();
  
  bool GetControlVisible();
  void SetControlVisible(bool visible);
  
private:
  
  QWidget* m_Widget;
  
};

}

#endif /*CHERRYQTEDITORPANE_H_*/
