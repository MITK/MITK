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

#include <QmitkDataStorageTreeInspector.h>

#include <QmitkDataStorageSimpleTreeModel.h>

QmitkDataStorageTreeInspector::QmitkDataStorageTreeInspector(QWidget* parent/* = nullptr*/)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  m_Controls.view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.view->setAlternatingRowColors(true);
  m_Controls.view->setHeaderHidden(true);
  m_Controls.view->setTextElideMode(Qt::ElideMiddle);

  m_StorageModel = new QmitkDataStorageSimpleTreeModel(this);

  m_Controls.view->setModel(m_StorageModel);
}

QAbstractItemView* QmitkDataStorageTreeInspector::GetView()
{
  return m_Controls.view;
};

const QAbstractItemView* QmitkDataStorageTreeInspector::GetView() const
{
  return m_Controls.view;
};

void QmitkDataStorageTreeInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Connector->SetView(m_Controls.view);

  m_Controls.view->expandAll();
}

void QmitkDataStorageTreeInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.view->setSelectionMode(mode);
}

QmitkDataStorageTreeInspector::SelectionMode QmitkDataStorageTreeInspector::GetSelectionMode() const
{
  return m_Controls.view->selectionMode();
};
