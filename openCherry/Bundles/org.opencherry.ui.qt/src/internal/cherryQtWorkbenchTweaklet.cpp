
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

#include "cherryQtWorkbenchTweaklet.h"
#include "cherryQtControlWidget.h"

#include <QApplication>
#include <QMainWindow>

#include "cherryQtShowViewDialog.h"
#include "cherryQtDisplay.h"

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>

namespace cherry {


int
QtWorkbenchTweaklet::RunEventLoop()
{
  QtDisplay display;

  // spin Qt event loop
  return QApplication::instance()->exec();
}

bool QtWorkbenchTweaklet::IsRunning()
{
  return QApplication::instance() != 0;
}

void* QtWorkbenchTweaklet::CreatePageComposite(void* p)
{
  QWidget* parent = static_cast<QWidget*>(p);
  QtControlWidget* pageArea = new QtControlWidget(parent, Shell::Pointer(0));
  pageArea->setObjectName("Page Composite");
  new QHBoxLayout(pageArea);
  if (qobject_cast<QMainWindow*>(parent) != 0)
    qobject_cast<QMainWindow*>(parent)->setCentralWidget(pageArea);
  else
    parent->layout()->addWidget(pageArea);

  // we have to enable visibility to get a proper layout (see bug #1654)
  pageArea->setVisible(true);

  return pageArea;
}


IDialog::Pointer
QtWorkbenchTweaklet::CreateStandardDialog(const std::string& dialogid)
{
  if (dialogid == DIALOG_ID_SHOW_VIEW)
    return IDialog::Pointer(new QtShowViewDialog(PlatformUI::GetWorkbench()->GetViewRegistry()));
  else
    return IDialog::Pointer(0);
}


}  // namespace cherry
