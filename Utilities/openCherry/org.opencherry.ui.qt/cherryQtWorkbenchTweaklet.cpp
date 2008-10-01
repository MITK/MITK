
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

#include "internal/cherryQtShowViewDialog.h"

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>

namespace cherry {


int
QtWorkbenchTweaklet::RunEventLoop()
{

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
  std::cout << "CreatePageComposite: parent is " << parent->metaObject()->className() << " with name " << qPrintable(parent->objectName()) << std::endl;
  QtControlWidget* pageArea = new QtControlWidget(parent);
  pageArea->setObjectName("Page Composite");
  new QHBoxLayout(pageArea);
  if (qobject_cast<QMainWindow*>(parent) != 0)
    qobject_cast<QMainWindow*>(parent)->setCentralWidget(pageArea);
  else
    parent->layout()->addWidget(pageArea);

  parent->layout()->update();

  QRect parentRect = parent->geometry();
  std::cout << "page composite parent geom: x = " << parentRect.x() << ", y = " << parentRect.y() << ", width = " << parentRect.width() << ", height = " << parentRect.height() << std::endl;
  parentRect = pageArea->geometry();
  std::cout << "page composite geom: x = " << parentRect.x() << ", y = " << parentRect.y() << ", width = " << parentRect.width() << ", height = " << parentRect.height() << std::endl;


  return pageArea;
}


IDialog::Pointer
QtWorkbenchTweaklet::CreateStandardDialog(const std::string& dialogid)
{
  if (dialogid == DIALOG_ID_SHOW_VIEW)
    return new QtShowViewDialog(PlatformUI::GetWorkbench()->GetViewRegistry());
  else
    return 0;
}


}  // namespace cherry
