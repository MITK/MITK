/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// data storage viewer test plugin
#include "QmitkDataStorageViewerTestView.h"

// berry
#include <berryIWorkbenchWindow.h>

// qt
#include <QHBoxLayout>

const std::string QmitkDataStorageViewerTestView::VIEW_ID = "org.mitk.views.datastorageviewertest";

void QmitkDataStorageViewerTestView::SetFocus()
{
  // nothing here
}

void QmitkDataStorageViewerTestView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_DataStorageDefaultListModel = new QmitkDataStorageDefaultListModel(this);
  m_DataStorageDefaultListModel->SetDataStorage(GetDataStorage());
  m_Controls.selectionListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.selectionListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.selectionListView->setAlternatingRowColors(true);
  m_Controls.selectionListView->setModel(m_DataStorageDefaultListModel);

  m_DataStorageDefaultListModel2 = new QmitkDataStorageDefaultListModel(this);
  m_DataStorageDefaultListModel2->SetDataStorage(GetDataStorage());
  m_Controls.selectionListView2->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.selectionListView2->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.selectionListView2->setAlternatingRowColors(true);
  m_Controls.selectionListView2->setModel(m_DataStorageDefaultListModel2);

  m_ModelViewSelectionConnector = std::make_unique<QmitkModelViewSelectionConnector>();
  m_ModelViewSelectionConnector->SetModel(m_DataStorageDefaultListModel);
  m_ModelViewSelectionConnector->SetView(m_Controls.selectionListView);
  m_ModelViewSelectionConnector->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());

  m_ModelViewSelectionConnector2 = std::make_unique<QmitkModelViewSelectionConnector>();
  m_ModelViewSelectionConnector2->SetModel(m_DataStorageDefaultListModel2);
  m_ModelViewSelectionConnector2->SetView(m_Controls.selectionListView2);
  m_ModelViewSelectionConnector2->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());

  connect(m_Controls.selectionProviderCheckBox, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider1(bool)));
  connect(m_Controls.selectionProviderCheckBox2, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider2(bool)));
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider1(bool checked)
{
  if (checked)
  {
    m_ModelViewSelectionConnector->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
  }
  else
  {
    m_ModelViewSelectionConnector->RemoveAsSelectionProvider();
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider2(bool checked)
{
  if (checked)
  {
    m_ModelViewSelectionConnector2->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
  }
  else
  {
    m_ModelViewSelectionConnector2->RemoveAsSelectionProvider();
  }
}
