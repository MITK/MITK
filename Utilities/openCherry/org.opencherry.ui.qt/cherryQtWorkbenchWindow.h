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

#ifndef CHERRYQTWORKBENCHWINDOW_H_
#define CHERRYQTWORKBENCHWINDOW_H_

#include <QtGui/QMainWindow>
#include <QtGui/QAction>
#include <QtGui/QTabWidget>

#include <vector>

#include <org.opencherry.ui/cherryIViewDescriptor.h>
#include <org.opencherry.ui/cherryWorkbenchWindow.h>

#include "cherryUiQtDll.h"

namespace cherry {

class QtShowViewAction;

class CHERRY_UI_QT QtWorkbenchWindow : public QMainWindow, public WorkbenchWindow
{
  Q_OBJECT

public:
  
  cherryClassMacro(QtWorkbenchWindow);
  
  QtWorkbenchWindow();
  ~QtWorkbenchWindow();
  
private:
  QMenu* m_ViewMenu;
  QMenu* m_HelpMenu;
  QTabWidget* m_EditorTabs;
  
  std::vector<QtShowViewAction*> m_ViewActions;
};


class QtShowViewAction : public QObject {
   Q_OBJECT
   
 public:
   QtShowViewAction(QtWorkbenchWindow* workbench, IViewDescriptor::Pointer descriptor);
   ~QtShowViewAction();
   
   QAction* GetQtAction();
   QDockWidget* GetDockWidget();
 
 private slots:
   void ShowView();
   
 private:
   QtWorkbenchWindow* m_Workbench;
   QAction* m_Action;
   IViewDescriptor::Pointer m_Descriptor;
   
   QDockWidget* m_Dock;
 };

}

#endif /*CHERRYQTWORKBENCHWINDOW_H_*/
