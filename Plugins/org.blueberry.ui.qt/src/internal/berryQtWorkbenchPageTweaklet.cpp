/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtWorkbenchPageTweaklet.h"

#include <internal/berryQtControlWidget.h>

#include <ui_berryQtStatusPart.h>

#include <QWidget>
#include <QHBoxLayout>

namespace berry
{

QtWorkbenchPageTweaklet::QtWorkbenchPageTweaklet()
{

}

QWidget* QtWorkbenchPageTweaklet::CreateClientComposite(QWidget* pageControl)
{
  QWidget* parent = static_cast<QWidget*>(pageControl);
  QtControlWidget* client = new QtControlWidget(parent, 0);
  client->setObjectName("ClientComposite");
  parent->layout()->addWidget(client);

  // we have to enable visibility to get a proper layout (see bug #1654)
  client->setVisible(true);

  return client;
}

QWidget* QtWorkbenchPageTweaklet::CreatePaneControl(QWidget* parent)
{
  QWidget* qParent = static_cast<QWidget*>(parent);
  QtControlWidget* control = new QtControlWidget(qParent, 0);
  // the object name is used in the global event filter to find
  // the pane control over which a mouse pressed event occured
  // --> do not change the object name
  control->setObjectName("PartPaneControl");

  return control;
}

Object::Pointer QtWorkbenchPageTweaklet::CreateStatusPart(QWidget* parent, const QString& title, const QString& msg)
{
  Ui::QtStatusPart statusPart;
  statusPart.setupUi(static_cast<QWidget*>(parent));
  statusPart.m_TitleLabel->setText(title);
  statusPart.m_DetailsLabel->setText(msg);

  return Object::Pointer(0);
}

IEditorPart::Pointer QtWorkbenchPageTweaklet::CreateErrorEditorPart(const QString&  /*partName*/, const QString&  /*msg*/)
{
  return IEditorPart::Pointer(0);
}

}
