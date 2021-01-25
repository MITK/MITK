/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAbstractNodeSelectionWidget.h"
#include "QmitkModelViewSelectionConnector.h"

QmitkAbstractNodeSelectionWidget::QmitkAbstractNodeSelectionWidget(QWidget* parent)
  : QWidget(parent)
  , m_InvalidInfo("Error. Select data.")
  , m_EmptyInfo("Empty. Make a selection.")
  , m_PopUpTitel("Select a data node")
  , m_PopUpHint("")
  , m_IsOptional(false)
  , m_SelectOnlyVisibleNodes(true)
  , m_DataStorageDeletedTag(0)
  , m_LastEmissionAllowance(true)
  , m_RecursionGuard(false)
{
}

QmitkAbstractNodeSelectionWidget::~QmitkAbstractNodeSelectionWidget()
{
  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNotNull())
  {
    // remove Listener for the data storage itself
    dataStorage->RemoveObserver(m_DataStorageDeletedTag);

    // remove "add node listener" from data storage
    dataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeAddedToStorage));

    // remove "remove node listener" from data storage
    dataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage));
  }

  for (auto& node : m_CurrentInternalSelection)
  {
    this->RemoveNodeObserver(node);
  }
}

QmitkAbstractNodeSelectionWidget::NodeList QmitkAbstractNodeSelectionWidget::GetSelectedNodes() const
{
  return this->CompileEmitSelection();
}

QmitkAbstractNodeSelectionWidget::ConstNodeStdVector QmitkAbstractNodeSelectionWidget::GetSelectedNodesStdVector() const
{
  auto result = this->GetSelectedNodes();
  return ConstNodeStdVector(result.begin(), result.end());
}

void QmitkAbstractNodeSelectionWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage == dataStorage)
  {
    return;
  }

  auto oldStorage = m_DataStorage.Lock();
  if (oldStorage.IsNotNull())
  {
    // remove Listener for the data storage itself
    oldStorage->RemoveObserver(m_DataStorageDeletedTag);

    // remove "add node listener" from old data storage
    oldStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeAddedToStorage));

    // remove "remove node listener" from old data storage
    oldStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage));
  }

  m_DataStorage = dataStorage;

  auto newStorage = m_DataStorage.Lock();

  if (newStorage.IsNotNull())
  {
    // add Listener for the data storage itself
    auto command = itk::SimpleMemberCommand<QmitkAbstractNodeSelectionWidget>::New();
    command->SetCallbackFunction(this, &QmitkAbstractNodeSelectionWidget::SetDataStorageDeleted);
    m_DataStorageDeletedTag = newStorage->AddObserver(itk::DeleteEvent(), command);

    // add "add node listener" for new data storage
    newStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeAddedToStorage));

    // add remove node listener for new data storage
    newStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage));
  }

  this->OnDataStorageChanged();

  this->HandleChangeOfInternalSelection({});
}

void QmitkAbstractNodeSelectionWidget::SetNodePredicate(const mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    this->OnNodePredicateChanged();

    NodeList newInternalNodes;

    for (auto& node : m_CurrentInternalSelection)
    {
      if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node))
      {
        newInternalNodes.append(node);
      }
    }

    if (!m_SelectOnlyVisibleNodes)
    {
      for (auto& node : m_CurrentExternalSelection)
      {
        if (!newInternalNodes.contains(node) && (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node)))
        {
          newInternalNodes.append(node);
        }
      }
    }

    this->HandleChangeOfInternalSelection(newInternalNodes);
  }
}

void QmitkAbstractNodeSelectionWidget::HandleChangeOfInternalSelection(NodeList newInternalSelection)
{
  if (!EqualNodeSelections(m_CurrentInternalSelection, newInternalSelection))
  {
    this->ReviseSelectionChanged(m_CurrentInternalSelection, newInternalSelection);

    this->SetCurrentInternalSelection(newInternalSelection);

    this->OnInternalSelectionChanged();

    auto newEmission = this->CompileEmitSelection();

    this->EmitSelection(newEmission);

    this->UpdateInfo();
  }
}

void QmitkAbstractNodeSelectionWidget::SetCurrentSelection(NodeList selectedNodes)
{
  if (!m_RecursionGuard)
  {
    m_CurrentExternalSelection = selectedNodes;

    auto dataStorage = m_DataStorage.Lock();
    NodeList newInternalSelection;
    for (const auto &node : qAsConst(selectedNodes))
    {
      if (dataStorage.IsNotNull() && dataStorage->Exists(node) && (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node)))
      {
        newInternalSelection.append(node);
      }
    }

    this->HandleChangeOfInternalSelection(newInternalSelection);
  }
}

const mitk::NodePredicateBase* QmitkAbstractNodeSelectionWidget::GetNodePredicate() const
{
  return m_NodePredicate;
}

QString QmitkAbstractNodeSelectionWidget::GetInvalidInfo() const
{
  return m_InvalidInfo;
}

QString QmitkAbstractNodeSelectionWidget::GetEmptyInfo() const
{
  return m_EmptyInfo;
}

QString QmitkAbstractNodeSelectionWidget::GetPopUpTitel() const
{
  return m_PopUpTitel;
}

QString QmitkAbstractNodeSelectionWidget::GetPopUpHint() const
{
  return m_PopUpHint;
}

bool QmitkAbstractNodeSelectionWidget::GetSelectionIsOptional() const
{
  return m_IsOptional;
}

bool QmitkAbstractNodeSelectionWidget::GetSelectOnlyVisibleNodes() const
{
  return m_SelectOnlyVisibleNodes;
}

void QmitkAbstractNodeSelectionWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  if (m_SelectOnlyVisibleNodes != selectOnlyVisibleNodes)
  {
    m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;

    auto newEmission = this->CompileEmitSelection();

    this->EmitSelection(newEmission);
  }
}

void QmitkAbstractNodeSelectionWidget::SetInvalidInfo(QString info)
{
  m_InvalidInfo = info;
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetEmptyInfo(QString info)
{
  m_EmptyInfo = info;
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetPopUpTitel(QString info)
{
  m_PopUpTitel = info;
}

void QmitkAbstractNodeSelectionWidget::SetPopUpHint(QString info)
{
  m_PopUpHint = info;
}

void QmitkAbstractNodeSelectionWidget::SetSelectionIsOptional(bool isOptional)
{
  m_IsOptional = isOptional;
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetDataStorageDeleted()
{
  this->OnDataStorageChanged();
  this->HandleChangeOfInternalSelection({});
}

void QmitkAbstractNodeSelectionWidget::ReviseSelectionChanged(const NodeList& /*oldInternalSelection*/, NodeList& /*newInternalSelection*/)
{
}

bool QmitkAbstractNodeSelectionWidget::AllowEmissionOfSelection(const NodeList& /*emissionCandidates*/) const
{
  return true;
}

void QmitkAbstractNodeSelectionWidget::EmitSelection(const NodeList& emissionCandidates)
{
  m_LastEmissionAllowance = this->AllowEmissionOfSelection(emissionCandidates);
  if (m_LastEmissionAllowance && !EqualNodeSelections(m_LastEmission, emissionCandidates))
  {
    m_RecursionGuard = true;
    emit CurrentSelectionChanged(emissionCandidates);
    m_RecursionGuard = false;
    m_LastEmission = emissionCandidates;
  }
}

void QmitkAbstractNodeSelectionWidget::SetCurrentInternalSelection(NodeList selectedNodes)
{
  for (auto& node : m_CurrentInternalSelection)
  {
    this->RemoveNodeObserver(node);
  }

  m_CurrentInternalSelection = selectedNodes;

  for (auto& node : m_CurrentInternalSelection)
  {
    this->AddNodeObserver(node);
  }
}

const QmitkAbstractNodeSelectionWidget::NodeList& QmitkAbstractNodeSelectionWidget::GetCurrentInternalSelection() const
{
  return m_CurrentInternalSelection;
}

const QmitkAbstractNodeSelectionWidget::NodeList& QmitkAbstractNodeSelectionWidget::GetCurrentExternalSelection() const
{
  return m_CurrentExternalSelection;
}

void QmitkAbstractNodeSelectionWidget::OnNodePredicateChanged()
{
}

void QmitkAbstractNodeSelectionWidget::OnDataStorageChanged()
{
}

void QmitkAbstractNodeSelectionWidget::OnInternalSelectionChanged()
{
}

void QmitkAbstractNodeSelectionWidget::NodeAddedToStorage(const mitk::DataNode* node)
{
  this->OnNodeAddedToStorage(node);
}

void QmitkAbstractNodeSelectionWidget::OnNodeAddedToStorage(const mitk::DataNode* /*node*/)
{
}

void QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage(const mitk::DataNode* node)
{
  this->OnNodeRemovedFromStorage(node);
  this->RemoveNodeFromSelection(node);
}

void QmitkAbstractNodeSelectionWidget::OnNodeRemovedFromStorage(const mitk::DataNode* /*node*/)
{
}

QmitkAbstractNodeSelectionWidget::NodeList QmitkAbstractNodeSelectionWidget::CompileEmitSelection() const
{
  NodeList result = m_CurrentInternalSelection;

  if (!m_SelectOnlyVisibleNodes)
  {
    for (const auto &node : m_CurrentExternalSelection)
    {
      if (!result.contains(node) && m_NodePredicate.IsNotNull() && !m_NodePredicate->CheckNode(node))
      {
        result.append(node);
      }
    }
  }

  return result;
}

void QmitkAbstractNodeSelectionWidget::RemoveNodeFromSelection(const mitk::DataNode* node)
{
  auto newSelection = m_CurrentInternalSelection;

  auto finding = std::find(std::begin(newSelection), std::end(newSelection), node);
  
  if (finding != std::end(newSelection))
  {
    newSelection.erase(finding);
    this->HandleChangeOfInternalSelection(newSelection);
  }
}

void QmitkAbstractNodeSelectionWidget::OnNodeModified(const itk::Object * caller, const itk::EventObject & event)
{
  if (itk::ModifiedEvent().CheckEvent(&event))
  {
    auto node = dynamic_cast<const mitk::DataNode*>(caller);

    if (node)
    {
      if (m_NodePredicate.IsNotNull() && !m_NodePredicate->CheckNode(node))
      {
        this->RemoveNodeFromSelection(node);
      }
      else
      {
        auto oldAllowance = m_LastEmissionAllowance;
        auto newEmission = this->CompileEmitSelection();
        auto nonConstNode = const_cast<mitk::DataNode*>(node);
        if (newEmission.contains(nonConstNode) && (oldAllowance != this->AllowEmissionOfSelection(newEmission)))
        {
          this->EmitSelection(newEmission);
          this->UpdateInfo();
        }
      }
    }
  }
}

void QmitkAbstractNodeSelectionWidget::AddNodeObserver(mitk::DataNode* node)
{
  if (node)
  {
    auto modifiedCommand = itk::MemberCommand<QmitkAbstractNodeSelectionWidget>::New();
    modifiedCommand->SetCallbackFunction(this, &QmitkAbstractNodeSelectionWidget::OnNodeModified);

    auto nodeModifiedObserverTag = node->AddObserver(itk::ModifiedEvent(), modifiedCommand);

    m_NodeObserverTags.insert(std::make_pair(node, nodeModifiedObserverTag));
  }
}

void QmitkAbstractNodeSelectionWidget::RemoveNodeObserver(mitk::DataNode* node)
{
  if (node)
  {
    auto finding = m_NodeObserverTags.find(node);
    if (finding != std::end(m_NodeObserverTags))
    {
      node->RemoveObserver(finding->second);
    }
    else
    {
      MITK_ERROR << "Selection widget is in a wrong state. A node should be removed from the internal selection but seems to have no observer. Node:" << node;
    }
    m_NodeObserverTags.erase(node);
  }
}
