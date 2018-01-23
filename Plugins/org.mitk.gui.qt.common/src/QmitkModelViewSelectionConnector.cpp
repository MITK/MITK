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

// mitk gui qt common plugin
#include "QmitkModelViewSelectionConnector.h"
#include "internal/QmitkDataNodeSelection.h"

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

// blueberry ui qt plugin
#include <berryINullSelectionListener.h>

QmitkModelViewSelectionConnector::QmitkModelViewSelectionConnector()
  : m_Model(nullptr)
  , m_View(nullptr)
  , m_SelectOnlyVisibleNodes(false)
{
  // nothing here
}

void QmitkModelViewSelectionConnector::SetModel(QmitkAbstractDataStorageModel* model)
{
  m_Model = model;
}

void QmitkModelViewSelectionConnector::SetView(QAbstractItemView* view)
{
  if (nullptr != m_View)
  {
    disconnect(m_View->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(ChangeModelSelection(const QItemSelection&, const QItemSelection&)));
  }

  m_View = view;
  if (nullptr != m_View)
  {
    connect(m_View->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(ChangeModelSelection(const QItemSelection&, const QItemSelection&)));
  }
}

void QmitkModelViewSelectionConnector::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;
}

void QmitkModelViewSelectionConnector::SetCurrentSelection(QList<mitk::DataNode::Pointer> selectedNodes)
{
  if (nullptr == m_Model || nullptr == m_View)
  {
    return;
  }

  // filter input nodes and return the modified input node list
  QList<mitk::DataNode::Pointer> filteredNodes = FilterNodeList(selectedNodes);

  bool equal = IsEqualToCurrentSelection(filteredNodes);
  if (equal)
  {
    return;
  }

  if (!m_SelectOnlyVisibleNodes)
  {
    // store the unmodified selection
    m_NonVisibleSelection = selectedNodes;
    // remove the nodes in the original selection that are already contained in the filtered input node list
    // this will keep the selection of the original nodes that are not presented by the current view unmodified, but allows to change the selection of the filtered nodes
    // later, the nodes of the 'm_NonVisibleSelection' member have to be added again to the list of selected (filtered) nodes, if a selection is sent from the current view (see 'ModelSelectionChanged')
    auto lambda = [&filteredNodes](mitk::DataNode::Pointer original) { return filteredNodes.contains(original); };
    m_NonVisibleSelection.erase(std::remove_if(m_NonVisibleSelection.begin(), m_NonVisibleSelection.end(), lambda), m_NonVisibleSelection.end());
  }

  // create new selection by retrieving the corresponding indices of the (filtered) nodes
  QItemSelection newCurrentSelection;
  for (const auto& node : filteredNodes)
  {
    QModelIndexList matched = m_Model->match(m_Model->index(0, 0), QmitkDataNodeRole, QVariant::fromValue<mitk::DataNode::Pointer>(node), 1, Qt::MatchRecursive);
    if (!matched.empty())
    {
      newCurrentSelection.select(matched.front(), matched.front());
    }
  }

  m_View->selectionModel()->select(newCurrentSelection, QItemSelectionModel::ClearAndSelect);
}

void QmitkModelViewSelectionConnector::ChangeModelSelection(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
  QList<mitk::DataNode::Pointer> nodes = GetSelectedNodes();

  if (!m_SelectOnlyVisibleNodes)
  {
    // add the non-visible nodes from the original selection
    nodes.append(m_NonVisibleSelection);
  }
  emit CurrentSelectionChanged(nodes);
}

QList<mitk::DataNode::Pointer> QmitkModelViewSelectionConnector::GetSelectedNodes() const
{
  if (nullptr == m_Model || nullptr == m_View)
  {
    return QList<mitk::DataNode::Pointer>();
  }

  QList<mitk::DataNode::Pointer> nodes;
  QModelIndexList selectedIndexes = m_View->selectionModel()->selectedIndexes();
  for (const auto& index : selectedIndexes)
  {
    QVariant qvariantDataNode = m_Model->data(index, QmitkDataNodeRole);
    if (qvariantDataNode.canConvert<mitk::DataNode::Pointer>())
    {
      nodes.push_back(qvariantDataNode.value<mitk::DataNode::Pointer>());
    }
  }
  return nodes;
}

QList<mitk::DataNode::Pointer> QmitkModelViewSelectionConnector::FilterNodeList(const QList<mitk::DataNode::Pointer>& nodes) const
{
  if (nodes.isEmpty())
  {
    return QList<mitk::DataNode::Pointer>();
  }

  if (nullptr == m_Model)
  {
    return nodes;
  }

  mitk::NodePredicateBase* nodePredicate = m_Model->GetNodePredicate();
  if (nullptr == nodePredicate)
  {
    // no filter set
    return nodes;
  }

  QList<mitk::DataNode::Pointer> result;
  for (const auto& node : nodes)
  {
    if (true == nodePredicate->CheckNode(node))
    {
      result.push_back(node);
    }
  }

  return result;
}

bool QmitkModelViewSelectionConnector::IsEqualToCurrentSelection(QList<mitk::DataNode::Pointer>& selectedNodes) const
{
  // get the currently selected nodes from the model
  QList<mitk::DataNode::Pointer> currentlySelectedNodes = GetSelectedNodes();

  if (currentlySelectedNodes.size() == selectedNodes.size())
  {
    // lambda to compare node pointer inside both lists
    auto lambda = [](mitk::DataNode::Pointer lhs, mitk::DataNode::Pointer rhs) { return lhs == rhs; };
    //bool equal = std::equal(filteredNodes.begin(), filteredNodes.end(), currentlySelectedNodes.begin(), currentlySelectedNodes.end(), lambda);
    return std::is_permutation(selectedNodes.begin(), selectedNodes.end(), currentlySelectedNodes.begin(), currentlySelectedNodes.end(), lambda);
  }

  return false;
}
