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

#include <QmitkDataStorageDefaultView.h>

// qt
#include <QHBoxLayout>

QmitkDataStorageDefaultView::QmitkDataStorageDefaultView(QWidget* parent/* = nullptr*/)
  : QmitkDataStorageAbstractView(parent)
{
  SetupUI();
}

QmitkDataStorageDefaultView::QmitkDataStorageDefaultView(mitk::DataStorage* dataStorage, QWidget* parent/* = nullptr*/)
  : QmitkDataStorageAbstractView(dataStorage, parent)
{
  SetupUI();
}

QmitkDataStorageDefaultView::~QmitkDataStorageDefaultView()
{
  // nothing here
}

void QmitkDataStorageDefaultView::SetupUI()
{
  m_DataStorageDefaultListModel = new QmitkDataStorageDefaultListModel(this);
  m_DataStorageListView = new QListView(this);

  m_DataStorageListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_DataStorageListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_DataStorageListView->setAlternatingRowColors(true);
  m_DataStorageListView->setModel(m_DataStorageDefaultListModel);

  SetModel(m_DataStorageDefaultListModel);
  SetView(m_DataStorageListView);

  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(m_DataStorageListView);

  setLayout(layout);
}

void QmitkDataStorageDefaultView::NodeAdded(const mitk::DataNode* node)
{
  // work around to force an update of the data nodes
  m_Model->SetDataStorage(m_DataStorage);
}

void QmitkDataStorageDefaultView::NodeChanged(const mitk::DataNode* node)
{
  // work around to force an update of the data nodes
  m_Model->SetDataStorage(m_DataStorage);
}

void QmitkDataStorageDefaultView::NodeRemoved(const mitk::DataNode* node)
{
  // work around to force an update of the data nodes
  m_Model->SetDataStorage(m_DataStorage);
}
