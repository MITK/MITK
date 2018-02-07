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

#include <QmitkDataStorageListViewWidget.h>

#include <QmitkDataStorageDefaultListModel.h>

QmitkDataStorageListViewWidget::QmitkDataStorageListViewWidget(QWidget* parent/* = nullptr*/)
  : QmitkAbstractDataStorageViewWidget(parent)
{
  m_Controls.setupUi(this);
  m_StorageModel = new QmitkDataStorageDefaultListModel(this);
}

QAbstractItemView* QmitkDataStorageListViewWidget::GetView()
{
  return m_Controls.view;
};

QAbstractItemView* QmitkDataStorageListViewWidget::GetView() const
{
  return m_Controls.view;
};

void QmitkDataStorageListViewWidget::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Controls.view->setModel(m_StorageModel);
}
