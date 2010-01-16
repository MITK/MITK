/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryQtWorkbenchPageTweaklet.h"

#include "berryQtControlWidget.h"

#include <ui_berryQtStatusPart.h>

#include <QWidget>
#include <QHBoxLayout>

namespace berry
{

void* QtWorkbenchPageTweaklet::CreateClientComposite(void* pageControl)
{
  QWidget* parent = static_cast<QWidget*>(pageControl);
  QtControlWidget* client = new QtControlWidget(parent, Shell::Pointer(0));
  client->setObjectName("Client Composite");
  parent->layout()->setContentsMargins(3, 3, 3, 3);
  parent->layout()->addWidget(client);

  // we have to enable visibility to get a proper layout (see bug #1654)
  client->setVisible(true);

  return client;
}

void* QtWorkbenchPageTweaklet::CreatePaneControl(void* parent)
{
  QWidget* qParent = static_cast<QWidget*>(parent);
  QtControlWidget* control = new QtControlWidget(qParent, Shell::Pointer(0));
  // the object name is used in the global event filter to find
  // the pane control over which a mouse pressed event occured
  // --> do not change the object name
  control->setObjectName("PartPaneControl");

  return control;
}

Object::Pointer QtWorkbenchPageTweaklet::CreateStatusPart(void* parent, const std::string& title, const std::string& msg)
{
  Ui::QtStatusPart statusPart;
  statusPart.setupUi(static_cast<QWidget*>(parent));
  statusPart.m_TitleLabel->setText(QString::fromStdString(title));
  statusPart.m_DetailsLabel->setText(QString::fromStdString(msg));

  return Object::Pointer(0);
}

IEditorPart::Pointer QtWorkbenchPageTweaklet::CreateErrorEditorPart(const std::string&  /*partName*/, const std::string&  /*msg*/)
{
  return IEditorPart::Pointer(0);
}

}
