/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk qt widgets module
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkNodeSelectionDialog.h>

// mitk core module
#include <mitkImage.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkRenderWindowLayerUtilities.h>

QmitkSynchronizedNodeSelectionWidget::QmitkSynchronizedNodeSelectionWidget(QWidget* parent)
  : QmitkAbstractNodeSelectionWidget(parent)
{
  m_Controls.setupUi(this);

  m_StorageModel = std::make_unique<QmitkRenderWindowDataNodeTableModel>(this);

  m_Controls.tableView->setModel(m_StorageModel.get());
  m_Controls.tableView->horizontalHeader()->setVisible(false);
  m_Controls.tableView->verticalHeader()->setVisible(false);
  m_Controls.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

  this->SetUpConnections();
  this->Initialize();
}

QmitkSynchronizedNodeSelectionWidget::~QmitkSynchronizedNodeSelectionWidget()
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNull())
  {
    return;
  }

  bool isSynchronized = this->IsSynchronized();
  if (!isSynchronized)
  {
    // If the model is not synchronizes,
    // we know that renderer-specific properties exist for all nodes.
    // These properties need to be removed from the nodes.
    auto allNodes = dataStorage->GetAll();
    for (auto& node : *allNodes)
    {
      // Delete the relevant renderer-specific properties for the node using the current base renderer.
      mitk::RenderWindowLayerUtilities::DeleteRenderWindowProperties(node, baseRenderer);
    }
  }
}

void QmitkSynchronizedNodeSelectionWidget::SetBaseRenderer(mitk::BaseRenderer* baseRenderer)
{
  if (m_BaseRenderer == baseRenderer)
  {
    // no need to do something
    return;
  }

  if (nullptr == baseRenderer)
  {
    return;
  }

  auto oldBaseRenderer = m_BaseRenderer.Lock();
  m_BaseRenderer = baseRenderer;

  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNull())
  {
    return;
  }

  bool isSynchronized = this->IsSynchronized();
  if (isSynchronized)
  {
    // If the model is synchronized,
    // all nodes use global / default properties.
    // No renderer-specific property lists should exist
    // so there is no need to transfer any property values.
  }
  else
  {
    // If the model is not synchronized,
    // we know that renderer-specific properties exist for all nodes.
    // These properties need to be removed from the nodes and
    // we need to transfer their values to new renderer-specific properties.
    auto allNodes = dataStorage->GetAll();
    for (auto& node : *allNodes)
    {
      // Set the relevant renderer-specific properties for the node using the new base renderer.
      // By transferring the values from the old property list,
      // the same property-state is kept when switching to another base renderer.
      mitk::RenderWindowLayerUtilities::TransferRenderWindowProperties(node, baseRenderer, oldBaseRenderer);
      // Delete the relevant renderer-specific properties for the node using the old base renderer.
      mitk::RenderWindowLayerUtilities::DeleteRenderWindowProperties(node, oldBaseRenderer);
    }
  }

  this->Initialize();
}

void QmitkSynchronizedNodeSelectionWidget::SetSelectAll(bool selectAll)
{
  if (selectAll == m_Controls.selectionModeCheckBox->isChecked())
  {
    // no need to do something
    return;
  }

  m_Controls.selectionModeCheckBox->setChecked(selectAll);
}

bool QmitkSynchronizedNodeSelectionWidget::GetSelectAll() const
{
  return m_Controls.selectionModeCheckBox->isChecked();
}

void QmitkSynchronizedNodeSelectionWidget::SetSynchronized(bool synchronize)
{
  if (synchronize == this->IsSynchronized())
  {
    // no need to do something
    return;
  }

  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNull())
  {
    return;
  }

  if (synchronize)
  {
    // set the base renderer of the model to nullptr, such that global properties are used
    m_StorageModel->SetCurrentRenderer(nullptr);

    // If the model is synchronized,
    // we know that the model was not synchronized before.
    // That means that all nodes use renderer-specific properties,
    // but now all nodes need global properties.
    // Thus we need to remove the renderer-specific properties of all nodes of the
    // datastorage.
    auto allNodes = dataStorage->GetAll();
    for (auto& node : *allNodes)
    {
      // For helper / hidden nodes:
      // If the node predicate does not match, do not remove the renderer-specific property
      // This is relevant for the crosshair data nodes, which are only visible inside their
      // corresponding render window.
      if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node))
      {
        // Delete the relevant renderer-specific properties for the node using the current base renderer.
        mitk::RenderWindowLayerUtilities::DeleteRenderWindowProperties(node, baseRenderer);
      }
    }
  }
  else
  {
    // set the base renderer of the model to current base renderer, such that renderer-specific properties are used
    m_StorageModel->SetCurrentRenderer(baseRenderer);

    // If the model is not synchronized anymore,
    // we know that the model was synchronized before.
    // That means that all nodes use global / default properties,
    // but now all nodes need renderer-specific properties.
    // Thus we need to modify the renderer-specific properties of all nodes of the
    // datastorage:
    //  - hide those nodes, which are not part of the newly selected nodes.
    //  - keep the property values of those nodes, which are part of the new selection AND
    //    have been selected before
    auto currentNodeSelection = this->GetCurrentInternalSelection();
    auto allNodes = dataStorage->GetAll();
    for (auto& node : *allNodes)
    {
      // check if the node is part of the current selection
      auto finding = std::find(std::begin(currentNodeSelection), std::end(currentNodeSelection), node);
      if (finding != std::end(currentNodeSelection)) // node found / part of the current selection
      {
        // Set the relevant renderer-specific properties for the node using the curent base renderer.
        // By transferring the values from the global / default property list,
        // the same property-state is kept when switching to non-synchronized mode.
        mitk::RenderWindowLayerUtilities::TransferRenderWindowProperties(node, baseRenderer, nullptr);
      }
      else
      {
        // If the node is not part of the selection, unset the relevant renderer-specific properties.
        // This will unset the "visible" and "layer" property for the renderer-specific property list and
        // hide the node for this renderer.
        // ATTENTION: This is required, since the synchronized property needs to be overwritten
        //            to make sure that the visibility is correctly set for the specific base renderer.
        this->DeselectNode(node);
      }
    }
  }

  // Since the synchronization might lead to a different node order depending on the layer properties, the render window
  // needs to be updated.
  // Explicitly request an update since a renderer-specific property change does not mark the node as modified.
  // see https://phabricator.mitk.org/T22322
  mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
}

bool QmitkSynchronizedNodeSelectionWidget::IsSynchronized() const
{
  return m_StorageModel->GetCurrentRenderer().IsNull();
}

void QmitkSynchronizedNodeSelectionWidget::OnModelUpdated()
{
  m_Controls.tableView->resizeRowsToContents();
  m_Controls.tableView->resizeColumnsToContents();
}

void QmitkSynchronizedNodeSelectionWidget::OnSelectionModeChanged(bool selectAll)
{
  emit SelectionModeChanged(selectAll);

  if (selectAll)
  {
    auto dataStorage = m_DataStorage.Lock();
    if (dataStorage.IsNull())
    {
      return;
    }

    auto allNodes = m_NodePredicate ? dataStorage->GetSubset(m_NodePredicate) : dataStorage->GetAll();
    NodeList currentSelection;
    for (auto& node : *allNodes)
    {
      currentSelection.append(node);
    }

    this->HandleChangeOfInternalSelection(currentSelection);
  }
}

void QmitkSynchronizedNodeSelectionWidget::OnEditSelection()
{
  QmitkNodeSelectionDialog* dialog = new QmitkNodeSelectionDialog(this);

  dialog->SetDataStorage(m_DataStorage.Lock());
  dialog->SetNodePredicate(m_NodePredicate);
  dialog->SetCurrentSelection(m_StorageModel->GetCurrentSelection());
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);

  m_Controls.changeSelectionButton->setChecked(true);
  if (dialog->exec())
  {
    m_Controls.selectionModeCheckBox->setChecked(false);
    emit SelectionModeChanged(false);

    auto selectedNodes = dialog->GetSelectedNodes();
    this->HandleChangeOfInternalSelection(selectedNodes);
  }

  m_Controls.changeSelectionButton->setChecked(false);

  delete dialog;
}

void QmitkSynchronizedNodeSelectionWidget::OnTableClicked(const QModelIndex& index)
{
  if (!index.isValid() || m_StorageModel.get() != index.model())
  {
    return;
  }

  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  QVariant dataNodeVariant = index.data(QmitkDataNodeRole);
  auto dataNode = dataNodeVariant.value<mitk::DataNode::Pointer>();

  if (index.column() == 1) // node visibility column
  {
    bool visibiliy = index.data(Qt::EditRole).toBool();
    m_StorageModel->setData(index, QVariant(!visibiliy), Qt::EditRole);
    return;
  }

  if (index.column() == 2) // reinit node column
  {
    this->ReinitNode(dataNode);
    return;
  }

  if (index.column() == 3) // remove node column
  {
    this->RemoveFromInternalSelection(dataNode);
    return;
  }
}

void QmitkSynchronizedNodeSelectionWidget::SetUpConnections()
{
  connect(m_StorageModel.get(), &QmitkRenderWindowDataNodeTableModel::ModelUpdated,
    this, &QmitkSynchronizedNodeSelectionWidget::OnModelUpdated);
  
  connect(m_Controls.selectionModeCheckBox, &QCheckBox::clicked,
    this, &QmitkSynchronizedNodeSelectionWidget::OnSelectionModeChanged);
  connect(m_Controls.changeSelectionButton, &QPushButton::clicked,
    this, &QmitkSynchronizedNodeSelectionWidget::OnEditSelection);

  connect(m_Controls.tableView, &QTableView::clicked,
    this, &QmitkSynchronizedNodeSelectionWidget::OnTableClicked);
}

void QmitkSynchronizedNodeSelectionWidget::Initialize()
{
  auto baseRenderer = m_BaseRenderer.Lock();
  auto dataStorage = m_DataStorage.Lock();
  m_StorageModel->SetDataStorage(dataStorage);
  m_StorageModel->SetCurrentRenderer(baseRenderer);

  if (baseRenderer.IsNull() || dataStorage.IsNull())
  {
    m_Controls.selectionModeCheckBox->setEnabled(false);
    m_Controls.changeSelectionButton->setEnabled(false);
    // reset the model if no data storage is defined
    m_StorageModel->removeRows(0, m_StorageModel->rowCount());
    return;
  }

  // Use the new data storage / node predicate to correctly set the list of
  // currently selected data nodes for the model.
  // If a new data storage or node predicate has been defined,
  // we switch to the "selectAll" mode and synchronize the selection for simplicity.
  // enable UI
  m_Controls.selectionModeCheckBox->setEnabled(true);
  m_Controls.changeSelectionButton->setEnabled(true);

  m_Controls.selectionModeCheckBox->setChecked(true);

  // set the base renderer of the model to nullptr, such that global properties are used (synchronized mode)
  m_StorageModel->SetCurrentRenderer(nullptr);
}

void QmitkSynchronizedNodeSelectionWidget::UpdateInfo()
{

}

void QmitkSynchronizedNodeSelectionWidget::OnDataStorageChanged()
{
  this->Initialize();
}

void QmitkSynchronizedNodeSelectionWidget::OnNodePredicateChanged()
{
  this->Initialize();
}

void QmitkSynchronizedNodeSelectionWidget::ReviseSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  bool isSynchronized = this->IsSynchronized();
  if (isSynchronized)
  {
    this->ReviseSynchronizedSelectionChanged(oldInternalSelection, newInternalSelection);
  }
  else
  {
    this->ReviseDesynchronizedSelectionChanged(oldInternalSelection, newInternalSelection);
  }

  // Since a new selection might have a different rendering tree the render windows
  // need to be updated.
  // Explicitly request an update since a renderer-specific property change does not mark the node as modified.
  // see https://phabricator.mitk.org/T22322
  mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
}

void QmitkSynchronizedNodeSelectionWidget::OnInternalSelectionChanged()
{
  m_StorageModel->SetCurrentSelection(this->GetCurrentInternalSelection());
}

bool QmitkSynchronizedNodeSelectionWidget::AllowEmissionOfSelection(const NodeList& /*emissionCandidates*/) const
{
  return this->IsSynchronized();
}

void QmitkSynchronizedNodeSelectionWidget::OnNodeAddedToStorage(const mitk::DataNode* node)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  // For helper / hidden nodes
  if (m_NodePredicate.IsNotNull() && !m_NodePredicate->CheckNode(node))
  {
    // If the node predicate does not match, do not add the node to the current selection.
    // Leave the visibility as it is.
    return;
  }

  // The selection mode determines if we want to show all nodes from the data storage
  // or use a local selected list of nodes.
  // We need to hide each new incoming data node, if we use a local selection,
  // since we do not want to show / select newly added nodes immediately.
  // We need to add the incoming node to our selection, if the selection mode check box
  // is checked.
  // We want to add the incoming node to our selection, if the node is a child node
  // of an already selected node.
  // Nodes added to the selection will be made visible.
  if (m_Controls.selectionModeCheckBox->isChecked() || this->IsParentNodeSelected(node))
  {
    auto currentSelection = this->GetCurrentInternalSelection();
    // Check if the nodes is already part of the internal selection.
    // That can happen if another render window already added the new node and sent out the new, updated
    // selection to be synchronized.
    auto finding = std::find(std::begin(currentSelection), std::end(currentSelection), node);
    if (finding != std::end(currentSelection)) // node found
    {
      // node already part of the selection
      return;
    }

    currentSelection.append(const_cast<mitk::DataNode*>(node));
    // This function will call 'QmitkSynchronizedNodeSelectionWidget::ReviseSelectionChanged'
    // which will take care of the visibility-property for newly added node.
    this->HandleChangeOfInternalSelection(currentSelection);
  }
  else
  {
    // If the widget is in "local-selection" state (selectionModeCheckBox unchecked),
    // the new incoming node needs to be hid.
    // Here it depends on the synchronization-state which properties need
    // to be modified.
    if (this->IsSynchronized())
    {
      // If the node will not be part of the new selection, hide the node.
      const_cast<mitk::DataNode*>(node)->SetVisibility(false);
    }
    else
    {
      // If the widget is not synchronized, all nodes use renderer-specific properties.
      // Thus we need to modify the renderer-specific properties of the node:
      //  - hide the node, which is not part of the selection
      this->DeselectNode(const_cast<mitk::DataNode*>(node));
    }
  }
}

void QmitkSynchronizedNodeSelectionWidget::OnNodeModified(const itk::Object* caller, const itk::EventObject& event)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  if (!itk::ModifiedEvent().CheckEvent(&event))
  {
    return;
  }

  auto node = dynamic_cast<const mitk::DataNode*>(caller);

  if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node))
  {
    auto currentSelection = this->GetCurrentInternalSelection();
    // check if the node to be modified is part of the current selection
    auto finding = std::find(std::begin(currentSelection), std::end(currentSelection), node);
    if (finding == std::end(currentSelection)) // node not found
    {
      // node not part of the selection
      return;
    }
    
    // We know that the node is relevant, but we don't know if the node modification was relevant
    // for the rendering. We just request an update here.
    // Explicitly request an update since a renderer-specific property change does not mark the node as modified.
    // see https://phabricator.mitk.org/T22322
    mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
    m_StorageModel->UpdateModelData();
  }
}

void QmitkSynchronizedNodeSelectionWidget::ReviseSynchronizedSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection)
{
  // If the model is synchronized, all nodes use global / default properties.
  // Thus we need to modify the global properties of the selection:
  //  - a) show those nodes, which are part of the new selection AND have not been
  //       selected before
  //  - b) keep the property values of those nodes, which are part of the new selection AND
  //       have been selected before
  //  - c) hide those nodes, which are part of the old selection AND
  //       have not been newly selected
  for (auto& node : newInternalSelection)
  {
    // check if the node is part of the old selection
    auto finding = std::find(std::begin(oldInternalSelection), std::end(oldInternalSelection), node);
    if (finding == std::end(oldInternalSelection)) // node not found
    {
      // If the node is part of the new selection and was not already part of the old selection,
      // set the relevant renderer-specific properties.
      // This will set the "visible" property for the global / default property list
      // and show the node for this renderer.
      node->SetVisibility(true); // item a)
    }
    // else: item b): node that was already selected before does not need to be modified
  }

  for (auto& node : oldInternalSelection)
  {
    // check if the node is part of the new selection
    auto finding = std::find(std::begin(newInternalSelection), std::end(newInternalSelection), node);
    if (finding == std::end(newInternalSelection)) // node not found
    {
      // If the node is not part of the new selection, hide the node.
      node->SetVisibility(false); // item c)
    }
    // else: item b): node that was already selected before does not need to be modified
  }
}

void QmitkSynchronizedNodeSelectionWidget::ReviseDesynchronizedSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  // If the model is not synchronized, all nodes need renderer-specific properties.
  // Thus we need to modify the renderer-specific properties of the selection:
  //  - a) set the renderer-specific properties of those nodes, which are part of the new selection AND
  //       have not been selected before (see 'SelectNode')
  //  - b) show those nodes, which are part of the new selection AND have not been
  //       selected before
  //  - c) keep the property values of those nodes, which are part of the new selection AND
  //       have been selected before
  //  - d) hide those nodes, which are part of the old selection AND
  //       have not been newly selected
  //  - e) set the renderer-specific properties of those nodes, which are part of the old selection AND
  //       have not been newly selected, to denote which nodes are selected
  for (auto& node : newInternalSelection)
  {
    // check if the node is part of the old selection
    auto finding = std::find(std::begin(oldInternalSelection), std::end(oldInternalSelection), node);
    if (finding == std::end(oldInternalSelection)) // node not found
    {
      // If the node is part of the new selection and was not already part of the old selection,
      // set the relevant renderer-specific properties.
      // This will set the "visible" and "layer" property for the renderer-specific property list
      // such that the global / default property list values are overwritten
      mitk::RenderWindowLayerUtilities::SetRenderWindowProperties(node, baseRenderer); // item a)
      // Explicitly set the visibility to true for selected nodes to show them in the render window.
      node->SetVisibility(true, baseRenderer); // item b)
    }
    // else: item c): node that was already selected before does not need to be modified
  }

  for (auto& node : oldInternalSelection)
  {
    // check if the node is part of the new selection
    auto finding = std::find(std::begin(newInternalSelection), std::end(newInternalSelection), node);
    if (finding == std::end(newInternalSelection)) // node not found
    {
      // If the node is not part of the new selection, unset the relevant renderer-specific properties.
      // This will unset the "visible" and "layer" property for the renderer-specific property list and
      // hide the node for this renderer.
      // ATTENTION: This is required, since the synchronized global property needs to be overwritten
      //            to make sure that the visibility is correctly set for the specific base renderer.
      this->DeselectNode(node); // item d) and e)
    }
    // else: item c): node that was already selected before does not need to be modified
  }
}

void QmitkSynchronizedNodeSelectionWidget::ReinitNode(const mitk::DataNode* dataNode)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  auto selectedImage = dynamic_cast<mitk::Image*>(dataNode->GetData());
  if (nullptr == selectedImage)
  {
    return;
  }

  auto boundingBoxPredicate = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false), baseRenderer));
  if (!boundingBoxPredicate->CheckNode(dataNode))
  {
    return;
  }

  mitk::RenderingManager::GetInstance()->InitializeView(baseRenderer->GetRenderWindow(), selectedImage->GetTimeGeometry());
}

void QmitkSynchronizedNodeSelectionWidget::RemoveFromInternalSelection(mitk::DataNode* dataNode)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  if (this->IsSynchronized())
  {
    // If the model is synchronized, all nodes use global / default properties.
    // Thus we need to modify the global property of the node.
    // Explicitly set the visibility to false for unselected nodes to hide them in the render window.
    dataNode->SetVisibility(false);
  }

  m_Controls.selectionModeCheckBox->setChecked(false);
  emit SelectionModeChanged(false);

  this->RemoveNodeFromSelection(dataNode);
}

bool QmitkSynchronizedNodeSelectionWidget::IsParentNodeSelected(const mitk::DataNode* dataNode) const
{
  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNull())
  {
    return false;
  }

  auto currentSelection = this->GetCurrentInternalSelection();
  auto parentNodes = dataStorage->GetSources(dataNode, m_NodePredicate, false);
  for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
  {
    const mitk::DataNode* parentNode = it->Value();
    auto finding = std::find(std::begin(currentSelection), std::end(currentSelection), parentNode);
    if (finding != std::end(currentSelection)) // parent node found
    {
      // at least one parent node is part of the selection
      return true;
    }
  }

  return false;
}

void QmitkSynchronizedNodeSelectionWidget::DeselectNode(mitk::DataNode* dataNode)
{
  auto baseRenderer = m_BaseRenderer.Lock();
  if (baseRenderer.IsNull())
  {
    return;
  }

  if (nullptr == dataNode)
  {
    return;
  }

  if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(dataNode))
  {
    // If the node should not be part of the selection, set the relevant renderer-specific properties.
    // This will set the "visible" and "layer" property for the renderer-specific property list,
    // such that the global / default property list values are overwritten.
    mitk::RenderWindowLayerUtilities::SetRenderWindowProperties(dataNode, baseRenderer);
    // Explicitly set the visibility to false for the node to hide them in the render window.
    dataNode->SetVisibility(false, baseRenderer);
  }
}
