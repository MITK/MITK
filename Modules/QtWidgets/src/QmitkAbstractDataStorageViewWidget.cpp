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

#include <QmitkAbstractDataStorageViewWidget.h>

QmitkAbstractDataStorageViewWidget::QmitkAbstractDataStorageViewWidget(QWidget* parent/* = nullptr*/)
  : QWidget(parent)
  , m_NodePredicate(nullptr)
{
  m_Connector = std::make_unique<QmitkModelViewSelectionConnector>();

  connect(m_Connector.get(), &QmitkModelViewSelectionConnector::CurrentSelectionChanged, this, &QmitkAbstractDataStorageViewWidget::OnSelectionChanged);
}

QmitkAbstractDataStorageViewWidget::~QmitkAbstractDataStorageViewWidget()
{
}

void QmitkAbstractDataStorageViewWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    m_DataStorage = dataStorage;

    if (!m_DataStorage.IsExpired())
    {
      this->Initialize();
    }
  }
}

void QmitkAbstractDataStorageViewWidget::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    this->Initialize();
  }
}

mitk::NodePredicateBase* QmitkAbstractDataStorageViewWidget::GetNodePredicate() const
{
  return m_NodePredicate;
}

QmitkAbstractDataStorageViewWidget::NodeList QmitkAbstractDataStorageViewWidget::GetSelectedNodes() const
{
  return m_Connector->GetSelectedNodes();
};

bool QmitkAbstractDataStorageViewWidget::GetSelectOnlyVisibleNodes() const
{
  return m_Connector->GetSelectOnlyVisibleNodes();
};

void QmitkAbstractDataStorageViewWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_Connector->SetSelectOnlyVisibleNodes(selectOnlyVisibleNodes);
};

void QmitkAbstractDataStorageViewWidget::SetCurrentSelection(NodeList selectedNodes)
{
  m_Connector->SetCurrentSelection(selectedNodes);
};

void QmitkAbstractDataStorageViewWidget::OnSelectionChanged(NodeList selectedNodes)
{
  emit CurrentSelectionChanged(selectedNodes);
};
