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

#include "cherryQtWorkbenchPageTweaklet.h"

#include "cherryQtControlWidget.h"

#include "internal/cherryQtErrorView.h"

#include <QWidget>
#include <QHBoxLayout>

namespace cherry
{

void* QtWorkbenchPageTweaklet::CreateClientComposite(void* pageControl)
{
  QWidget* parent = static_cast<QWidget*>(pageControl);
  QtControlWidget* client = new QtControlWidget(parent);
  client->setObjectName("Client Composite");
  parent->layout()->addWidget(client);

  QRect parentRect = parent->geometry();
  std::cout << "client composite parent geom: x = " << parentRect.x() << ", y = " << parentRect.y() << ", width = " << parentRect.width() << ", height = " << parentRect.height() << std::endl;
  parentRect = client->geometry();
  std::cout << "client composite geom: x = " << parentRect.x() << ", y = " << parentRect.y() << ", width = " << parentRect.width() << ", height = " << parentRect.height() << std::endl;

  std::cout << "CreateClientComposite: client composite <-- " << qPrintable(parent->objectName());
  for (parent = parent->parentWidget(); parent != 0; parent = parent->parentWidget())
    std::cout << " <-- " << qPrintable(parent->objectName());
  std::cout << std::endl;

  //new QHBoxLayout(client);
  return client;
}

void* QtWorkbenchPageTweaklet::CreatePaneControl(void* parent)
{
  QWidget* qParent = static_cast<QWidget*>(parent);
  QtControlWidget* control = new QtControlWidget(qParent);
  control->setObjectName("Pane Control");
  //qParent->layout()->addWidget(control);

  std::cout << "CreatePaneControl: Pane Control <--" << qPrintable(qParent->objectName());
  for (qParent = qParent->parentWidget(); qParent != 0; qParent = qParent->parentWidget())
    std::cout << " <-- " << qPrintable(qParent->objectName());
  std::cout << std::endl;

  return control;
}

IViewPart::Pointer QtWorkbenchPageTweaklet::CreateErrorViewPart(const std::string& partName, const std::string& msg)
{
  QtErrorView::Pointer part = new QtErrorView();
  //part->SetPartName(partName);
  part->SetErrorMsg(msg);
  return part;
}

IEditorPart::Pointer QtWorkbenchPageTweaklet::CreateErrorEditorPart(const std::string& partName, const std::string& msg)
{
  return 0;
}

}
