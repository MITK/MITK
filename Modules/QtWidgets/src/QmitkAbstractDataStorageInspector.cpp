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

#include <QmitkAbstractDataStorageInspector.h>

QmitkAbstractDataStorageInspector::QmitkAbstractDataStorageInspector(QWidget* parent/* = nullptr*/)
  : QWidget(parent)
  , m_NodePredicate(nullptr)
{
  m_Connector = std::make_unique<QmitkModelViewSelectionConnector>();

  connect(m_Connector.get(), &QmitkModelViewSelectionConnector::CurrentSelectionChanged, this, &QmitkAbstractDataStorageInspector::OnSelectionChanged);
}

QmitkAbstractDataStorageInspector::~QmitkAbstractDataStorageInspector()
{
}

void QmitkAbstractDataStorageInspector::SetDataStorage(mitk::DataStorage* dataStorage)
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

void QmitkAbstractDataStorageInspector::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    this->Initialize();
  }
}

mitk::NodePredicateBase* QmitkAbstractDataStorageInspector::GetNodePredicate() const
{
  return m_NodePredicate;
}

QmitkAbstractDataStorageInspector::NodeList QmitkAbstractDataStorageInspector::GetSelectedNodes() const
{
  return m_Connector->GetSelectedNodes();
};

bool QmitkAbstractDataStorageInspector::GetSelectOnlyVisibleNodes() const
{
  return m_Connector->GetSelectOnlyVisibleNodes();
};

void QmitkAbstractDataStorageInspector::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_Connector->SetSelectOnlyVisibleNodes(selectOnlyVisibleNodes);
};

void QmitkAbstractDataStorageInspector::SetCurrentSelection(NodeList selectedNodes)
{
  m_Connector->SetCurrentSelection(selectedNodes);
};

void QmitkAbstractDataStorageInspector::OnSelectionChanged(NodeList selectedNodes)
{
  emit CurrentSelectionChanged(selectedNodes);
};
