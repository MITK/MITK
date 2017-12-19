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

#include "QmitkDataStorageAbstractView.h"

// qtwidgets
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

QmitkDataStorageAbstractView::QmitkDataStorageAbstractView(QWidget* parent/* = nullptr*/)
  : QWidget(parent)
  , m_DataStorage(nullptr)
  , m_NodePredicate(nullptr)
  , m_SelectOnlyVisibleNodes(false)
  , m_Model(nullptr)
  , m_View(nullptr)
{
  // nothing here
}

QmitkDataStorageAbstractView::QmitkDataStorageAbstractView(mitk::DataStorage* dataStorage, QWidget* parent/* = nullptr*/)
  : QWidget(parent)
  , m_DataStorage(nullptr)
  , m_NodePredicate(nullptr)
  , m_SelectOnlyVisibleNodes(false)
  , m_Model(nullptr)
  , m_View(nullptr)
{
  SetDataStorage(dataStorage);
}

QmitkDataStorageAbstractView::~QmitkDataStorageAbstractView()
{
  if (nullptr != m_DataStorage)
  {
    // remove listener from old data storage
    m_DataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeAdded));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeRemoved));
    m_DataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeChanged));
  }
}

void QmitkDataStorageAbstractView::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage == dataStorage)
  {
    return;
  }

  if (nullptr != m_DataStorage)
  {
    // remove listener from old data storage
    m_DataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeAdded));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeRemoved));
    m_DataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeChanged));
  }

  m_DataStorage = dataStorage;

  if (nullptr != dataStorage)
  {
    // add listener for new data storage
    m_DataStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeAdded));
    m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeRemoved));
    m_DataStorage->ChangedNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataStorageAbstractView, const mitk::DataNode*>(this, &QmitkDataStorageAbstractView::NodeChanged));

  }
  // update model if the data storage has been changed
  DataStorageChanged();
}

void QmitkDataStorageAbstractView::DataStorageChanged()
{
  if (nullptr == m_Model)
  {
    return;
  }

  m_Model->SetDataStorage(m_DataStorage);
}

void QmitkDataStorageAbstractView::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate == nodePredicate)
  {
    return;
  }

  m_NodePredicate = nodePredicate;
  // update model if the node predicate has been changed
  NodePredicateChanged();
}

void QmitkDataStorageAbstractView::NodePredicateChanged()
{
  m_Model->SetNodePredicate(m_NodePredicate);
}

void QmitkDataStorageAbstractView::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;
}

void QmitkDataStorageAbstractView::SetCurrentSelection(QList<mitk::DataNode::Pointer> selectedNodes)
{
  // filter input nodes and return the modified input node list
  QList<mitk::DataNode::Pointer> filteredNodes = FilterNodeList(selectedNodes);
  // get the currently selected nodes from the model
  QList<mitk::DataNode::Pointer> currentlySelectedNodes = GetSelectedNodes();

  // compare currently selected nodes with filtered input node list
  if (currentlySelectedNodes.size() == filteredNodes.size())
  {
    // lambda to compare node pointer inside both lists
    auto lambda = [](mitk::DataNode::Pointer lhs, mitk::DataNode::Pointer rhs) { return lhs == rhs; };
    bool equal = std::equal(filteredNodes.begin(), filteredNodes.end(), currentlySelectedNodes.begin(), currentlySelectedNodes.end(), lambda);
    if (equal)
    {
      // node lists are equal, no need to update the selection model
      return;
    }
  }

  if (!m_SelectOnlyVisibleNodes)
  {
    // store the unmodified selection
    m_NonVisibleSelection = selectedNodes;
    // remove the nodes in the original selection that are already contained in the filtered input node list
    // this will keep the selection of the original nodes that are not presented by the current view unmodified, but allows to change the selection of the filtered nodes
    // later, the nodes of the 'm_NonVisibleSelection' member have to be added again to the list of selected (filtered) nodes, if a selection is sent from the current view (see 'ModelSelectionChanged')
    auto lambda = [&filteredNodes](mitk::DataNode::Pointer original) { return filteredNodes.contains(original); };
    m_NonVisibleSelection.erase(std::remove_if(m_NonVisibleSelection.begin(), m_NonVisibleSelection.end(), lambda),
                                m_NonVisibleSelection.end());
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

void QmitkDataStorageAbstractView::ModelSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
  QList<mitk::DataNode::Pointer> nodes = GetSelectedNodes();

  if (!m_SelectOnlyVisibleNodes)
  {
    // add the non-visible nodes from the original selection
    nodes.append(m_NonVisibleSelection);
  }
  emit CurrentSelectionChanged(nodes);
}

void QmitkDataStorageAbstractView::SetModel(QmitkIDataStorageViewModel* model)
{
  m_Model = model;
  DataStorageChanged();
}

void QmitkDataStorageAbstractView::SetView(QAbstractItemView* view)
{
  if (nullptr != m_View)
  {
    disconnect(m_View->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(ModelSelectionChanged(const QItemSelection&, const QItemSelection&)));
  }

  m_View = view;
  connect(m_View->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(ModelSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void QmitkDataStorageAbstractView::NodeAdded(const mitk::DataNode* node)
{
  // nothing here; can be overwritten in subclass
}

void QmitkDataStorageAbstractView::NodeChanged(const mitk::DataNode* node)
{
  // nothing here; can be overwritten in subclass
}

void QmitkDataStorageAbstractView::NodeRemoved(const mitk::DataNode* node)
{
  // nothing here; can be overwritten in subclass
}

QList<mitk::DataNode::Pointer> QmitkDataStorageAbstractView::GetSelectedNodes() const
{
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

QList<mitk::DataNode::Pointer> QmitkDataStorageAbstractView::FilterNodeList(const QList<mitk::DataNode::Pointer>& nodes) const
{
  if (nodes.isEmpty())
  {
    return QList<mitk::DataNode::Pointer>();
  }

  if (nullptr == m_NodePredicate)
  {
    // no filter set
    return nodes;
  }

  QList<mitk::DataNode::Pointer> result;
  for (const auto& node : nodes)
  {
    if (true == m_NodePredicate->CheckNode(node))
    {
      result.push_back(node);
    }
  }

  return result;
}
