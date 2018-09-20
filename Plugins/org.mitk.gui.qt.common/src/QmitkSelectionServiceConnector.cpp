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
#include "QmitkSelectionServiceConnector.h"
#include "internal/QmitkDataNodeSelection.h"

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

// blueberry ui qt plugin
#include <berryINullSelectionListener.h>

QmitkSelectionServiceConnector::QmitkSelectionServiceConnector()
  : m_SelectionService(nullptr)
  , m_SelectionProvider(nullptr)
{
  m_DataNodeItemModel = std::make_shared<QmitkDataNodeItemModel>();
  m_DataNodeSelectionModel = std::make_shared<QItemSelectionModel>(m_DataNodeItemModel.get());
}

QmitkSelectionServiceConnector::~QmitkSelectionServiceConnector()
{
  RemovePostSelectionListener();
  RemoveAsSelectionProvider();
}

void QmitkSelectionServiceConnector::AddPostSelectionListener(berry::ISelectionService* selectionService)
{
  if (nullptr == selectionService)
  {
    return;
  }

  m_SelectionService = selectionService;
  m_BerrySelectionListener.reset(new berry::NullSelectionChangedAdapter<QmitkSelectionServiceConnector>(this, &QmitkSelectionServiceConnector::OnServiceSelectionChanged));
  m_SelectionService->AddPostSelectionListener(m_BerrySelectionListener.get());
}

void QmitkSelectionServiceConnector::RemovePostSelectionListener()
{
  if (nullptr == m_SelectionService)
  {
    return;
  }

  m_SelectionService->RemovePostSelectionListener(m_BerrySelectionListener.get());
  m_SelectionService = nullptr;
}

void QmitkSelectionServiceConnector::SetAsSelectionProvider(QmitkDataNodeSelectionProvider* selectionProvider)
{
  m_SelectionProvider = selectionProvider;
}

void QmitkSelectionServiceConnector::RemoveAsSelectionProvider()
{
  m_SelectionProvider = nullptr;
}

void QmitkSelectionServiceConnector::ChangeServiceSelection(QList<mitk::DataNode::Pointer> nodes)
{
  if (nullptr == m_SelectionProvider)
  {
    return;
  }

  m_SelectionProvider->SetItemSelectionModel(m_DataNodeSelectionModel.get());

  if (nodes.empty())
  {
    m_DataNodeSelectionModel->clearSelection();
    m_DataNodeItemModel->clear();
  }
  else
  {
    m_DataNodeItemModel->clear();
    // fill the temporary helper data node item model with the nodes to select
    for (const auto& node : nodes)
    {
      m_DataNodeItemModel->AddDataNode(node);
    }

    m_DataNodeSelectionModel->select(QItemSelection(m_DataNodeItemModel->index(0, 0), m_DataNodeItemModel->index(nodes.size() - 1, 0)), QItemSelectionModel::ClearAndSelect);
  }
}

void QmitkSelectionServiceConnector::OnServiceSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcePart, const berry::ISelection::ConstPointer& selection)
{
  if (sourcePart.IsNull())
  {
    return;
  }

  QList<mitk::DataNode::Pointer> nodes;
  if (selection.IsNull())
  {
    emit ServiceNullSelection(sourcePart);
    return;
  }

  // transform valid selection to DataNodeSelection, which allows to retrieve the selected nodes
  mitk::DataNodeSelection::ConstPointer dataNodeSelection = selection.Cast<const mitk::DataNodeSelection>();
  if (dataNodeSelection.IsNull())
  {
    // propagate an empty list
    nodes = QList<mitk::DataNode::Pointer>();
  }
  else
  {
    nodes = QList<mitk::DataNode::Pointer>::fromStdList(dataNodeSelection->GetSelectedDataNodes());
  }

  // send new (possibly empty) list of selected nodes
  emit ServiceSelectionChanged(nodes);
}
