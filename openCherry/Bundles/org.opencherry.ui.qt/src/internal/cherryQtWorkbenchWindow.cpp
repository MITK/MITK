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

#include "cherryQtWorkbenchWindow.h"

#include "cherryQtControlWidget.h"
#include "cherryQtPerspectiveSwitcher.h"

#include <QWidget>
#include <QMainWindow>
#include <QHBoxLayout>

namespace cherry
{

QtWorkbenchWindow::QtWorkbenchWindow(int number) :
  WorkbenchWindow(number)
{
}

void* QtWorkbenchWindow::CreatePageComposite(void* p)
{
  QWidget* parent = static_cast<QWidget*> (p);
  QtControlWidget* pageArea = new QtControlWidget(parent, Shell::Pointer(0));
  pageArea->setObjectName("Page Composite");
  new QHBoxLayout(pageArea);
  if (qobject_cast<QMainWindow*> (parent) != 0)
    qobject_cast<QMainWindow*> (parent)->setCentralWidget(pageArea);
  else
    parent->layout()->addWidget(pageArea);

  // we have to enable visibility to get a proper layout (see bug #1654)
  pageArea->setVisible(true);

  pageComposite = pageArea;
  return pageArea;
}

void QtWorkbenchWindow::CreateDefaultContents(Shell::Pointer shell)
{
  QMainWindow* mainWindow = static_cast<QMainWindow*>(shell->GetControl());

  if (GetWindowConfigurer()->GetShowPerspectiveBar() && mainWindow)
  {
    mainWindow->addToolBar(new QtPerspectiveSwitcher(IWorkbenchWindow::Pointer(this)));
  }

  // Create the client composite area (where page content goes).
  CreatePageComposite(shell->GetControl());
}

}
