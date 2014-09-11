/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "ViewNavigatorView.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string ViewNavigatorView::VIEW_ID = "org.mitk.views.viewnavigatorview";

void ViewNavigatorView::SetFocus()
{

}

void ViewNavigatorView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );



//  candyStore = new QDockWidget("Candy Store");
//  candyStore->setWidget(new QmitkViewNavigatorWidget());
//  candyStore->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
//  candyStore->setVisible(false);
//  candyStore->setObjectName("Candy Store");
//  mainWindow->addDockWidget(Qt::LeftDockWidgetArea, candyStore);
}

void ViewNavigatorView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{

}
