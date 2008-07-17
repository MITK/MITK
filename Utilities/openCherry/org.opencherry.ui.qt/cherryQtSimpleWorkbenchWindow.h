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

#include "cherryUiQtDll.h"
#include "internal/cherryQtShowViewAction.h"

namespace cherry {


class CHERRY_UI_QT QtWorkbenchWindow : public QMainWindow, public WorkbenchWindow
{
  Q_OBJECT

public:

  cherryClassMacro(QtWorkbenchWindow);

  QtWorkbenchWindow(int number);
  ~QtWorkbenchWindow();

  void* GetMenuManager();

  int OpenImpl();

protected:

  friend class QtWorkbenchPageTweaklet;

  void CreateDefaultContents(void* shell);
  void* GetPageComposite();

private:

  std::vector<QtShowViewAction*> m_ViewActions;

};


}

#endif /*CHERRYQTSIMPLEWORKBENCHWINDOW_H_*/
