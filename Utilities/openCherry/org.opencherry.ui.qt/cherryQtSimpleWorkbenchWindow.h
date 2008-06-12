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

#ifndef CHERRYQTSIMPLEWORKBENCHWINDOW_H_
#define CHERRYQTSIMPLEWORKBENCHWINDOW_H_

#include <QMainWindow>

#include <vector>

#include <cherryIViewDescriptor.h>
#include <cherryWorkbenchWindow.h>
#include <internal/cherryWorkbenchPage.h>

#include "cherryUiQtDll.h"
#include "internal/cherryQtShowViewAction.h"

namespace cherry {


class CHERRY_UI_QT QtWorkbenchWindow : public QMainWindow, public WorkbenchWindow
{
  Q_OBJECT

public:
  
  cherryClassMacro(QtWorkbenchWindow);
  
  QtWorkbenchWindow();
  ~QtWorkbenchWindow();
  
  IWorkbenchPage::Pointer GetActivePage();
  
  void* GetMenuManager();
  
protected:
  
  void CreateDefaultContents(void* shell);
  
private:
  
  std::vector<QtShowViewAction*> m_ViewActions;
  
  // WARNING: using internal UI class
  WorkbenchPage::Pointer m_WorkbenchPage;
};


}

#endif /*CHERRYQTSIMPLEWORKBENCHWINDOW_H_*/
