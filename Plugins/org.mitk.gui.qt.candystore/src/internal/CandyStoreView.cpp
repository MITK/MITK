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
#include "CandyStoreView.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string CandyStoreView::VIEW_ID = "org.mitk.views.candystoreview";

void CandyStoreView::SetFocus()
{

}

void CandyStoreView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );



//  candyStore = new QDockWidget("Candy Store");
//  candyStore->setWidget(new QmitkCandyStoreWidget());
//  candyStore->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
//  candyStore->setVisible(false);
//  candyStore->setObjectName("Candy Store");
//  mainWindow->addDockWidget(Qt::LeftDockWidgetArea, candyStore);
}

void CandyStoreView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{

}
