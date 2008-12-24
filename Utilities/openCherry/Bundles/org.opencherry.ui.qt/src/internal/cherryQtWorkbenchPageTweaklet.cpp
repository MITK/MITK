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
#include "cherryQtErrorView.h"

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

  // we have to enable visibility to get a proper layout (see bug #1654)
  client->setVisible(true);

  return client;
}

void* QtWorkbenchPageTweaklet::CreatePaneControl(void* parent)
{
  QWidget* qParent = static_cast<QWidget*>(parent);
  QtControlWidget* control = new QtControlWidget(qParent);
  control->setObjectName("Pane Control");

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
