/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  auto   client = new QtControlWidget(parent, nullptr);
  client->setObjectName("ClientComposite");
  parent->layout()->addWidget(client);

  // we have to enable visibility to get a proper layout (see bug #1654)
  client->setVisible(true);

  return client;
}

QWidget* QtWorkbenchPageTweaklet::CreatePaneControl(QWidget* parent)
{
  QWidget* qParent = static_cast<QWidget*>(parent);
  auto   control = new QtControlWidget(qParent, nullptr);
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

  return Object::Pointer(nullptr);
}

IEditorPart::Pointer QtWorkbenchPageTweaklet::CreateErrorEditorPart(const QString&  /*partName*/, const QString&  /*msg*/)
{
  return IEditorPart::Pointer(nullptr);
}

}
