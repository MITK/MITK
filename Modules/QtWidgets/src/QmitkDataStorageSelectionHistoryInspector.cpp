/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageSelectionHistoryInspector.h>

#include "QmitkDataStorageHistoryModel.h"

QmitkDataStorageSelectionHistoryInspector::QmitkDataStorageSelectionHistoryInspector(QWidget* parent/* = nullptr*/)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  m_Controls.view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.view->setAlternatingRowColors(true);

  m_Overlay = new QmitkSimpleTextOverlayWidget(this);
  m_Overlay->setVisible(false);
  m_Overlay->SetOverlayText(QStringLiteral("<font class=\"normal\"><p style=\"text-align:center\">No history available yet.</p></center></font>"));

  m_StorageModel = new QmitkDataStorageHistoryModel(this);

  m_Controls.view->setModel(m_StorageModel);
}

QAbstractItemView* QmitkDataStorageSelectionHistoryInspector::GetView()
{
  return m_Controls.view;
}

const QAbstractItemView* QmitkDataStorageSelectionHistoryInspector::GetView() const
{
  return m_Controls.view;
}

void QmitkDataStorageSelectionHistoryInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Connector->SetView(m_Controls.view);

  m_Overlay->setVisible(m_StorageModel->rowCount() == 0);
}

void QmitkDataStorageSelectionHistoryInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.view->setSelectionMode(mode);
}

QmitkDataStorageSelectionHistoryInspector::SelectionMode QmitkDataStorageSelectionHistoryInspector::GetSelectionMode() const
{
  return m_Controls.view->selectionMode();
}

void QmitkDataStorageSelectionHistoryInspector::AddNodeToHistory(mitk::DataNode* node)
{
    QmitkDataStorageHistoryModel::AddNodeToHistory(node);
}

void QmitkDataStorageSelectionHistoryInspector::ResetHistory()
{
    QmitkDataStorageHistoryModel::ResetHistory();
}
