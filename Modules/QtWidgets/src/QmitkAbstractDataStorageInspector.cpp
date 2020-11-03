/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

void QmitkAbstractDataStorageInspector::SetNodePredicate(const mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    this->Initialize();
  }
}

const mitk::NodePredicateBase* QmitkAbstractDataStorageInspector::GetNodePredicate() const
{
  return m_NodePredicate;
}

QmitkAbstractDataStorageInspector::NodeList QmitkAbstractDataStorageInspector::GetSelectedNodes() const
{
  return m_Connector->GetSelectedNodes();
}

bool QmitkAbstractDataStorageInspector::GetSelectOnlyVisibleNodes() const
{
  return m_Connector->GetSelectOnlyVisibleNodes();
}

void QmitkAbstractDataStorageInspector::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_Connector->SetSelectOnlyVisibleNodes(selectOnlyVisibleNodes);
}

void QmitkAbstractDataStorageInspector::SetCurrentSelection(NodeList selectedNodes)
{
  m_Connector->SetCurrentSelection(selectedNodes);
}

void QmitkAbstractDataStorageInspector::OnSelectionChanged(NodeList selectedNodes)
{
  emit CurrentSelectionChanged(selectedNodes);
}
