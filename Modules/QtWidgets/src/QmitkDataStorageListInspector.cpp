/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageListInspector.h>

#include <QmitkDataStorageDefaultListModel.h>

QmitkDataStorageListInspector::QmitkDataStorageListInspector(QWidget* parent/* = nullptr*/)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  m_Controls.view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.view->setAlternatingRowColors(true);

  m_StorageModel = new QmitkDataStorageDefaultListModel(this);

  m_Controls.view->setModel(m_StorageModel);
}

QAbstractItemView* QmitkDataStorageListInspector::GetView()
{
  return m_Controls.view;
}

const QAbstractItemView* QmitkDataStorageListInspector::GetView() const
{
  return m_Controls.view;
}

void QmitkDataStorageListInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Connector->SetView(m_Controls.view);
}

void QmitkDataStorageListInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.view->setSelectionMode(mode);
}

QmitkDataStorageListInspector::SelectionMode QmitkDataStorageListInspector::GetSelectionMode() const
{
  return m_Controls.view->selectionMode();
}
