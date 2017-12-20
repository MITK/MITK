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
#include "QmitkDataStorageSelectionConnector.h"
#include "internal/QmitkDataNodeSelection.h"

// org.blueberry.ui.qt
#include <berryINullSelectionListener.h>

QmitkDataStorageSelectionConnector::QmitkDataStorageSelectionConnector(QmitkDataStorageAbstractView* dataStorageAbstractView)
	: m_DataStorageAbstractView(dataStorageAbstractView)
  , m_SelectionService(nullptr)
  , m_SelectionProvider(nullptr)
{
  m_DataNodeItemModel = std::make_shared<QmitkDataNodeItemModel>();
  m_DataNodeSelectionModel = std::make_shared<QItemSelectionModel>(m_DataNodeItemModel.get());
}

QmitkDataStorageSelectionConnector::~QmitkDataStorageSelectionConnector()
{
  RemovePostSelectionListener();
  RemoveAsSelectionProvider();
}

void QmitkDataStorageSelectionConnector::AddPostSelectionListener(berry::ISelectionService* selectionService)
{
  if (nullptr == selectionService)
  {
    return;
  }

	m_SelectionService = selectionService;
	m_BerrySelectionListener.reset(new berry::NullSelectionChangedAdapter<QmitkDataStorageSelectionConnector>(this, &QmitkDataStorageSelectionConnector::GlobalSelectionChanged));
	m_SelectionService->AddPostSelectionListener(m_BerrySelectionListener.get());
}

void QmitkDataStorageSelectionConnector::RemovePostSelectionListener()
{
	if (nullptr == m_SelectionService)
	{
		return;
	}

	m_SelectionService->RemovePostSelectionListener(m_BerrySelectionListener.get());
	m_SelectionService = nullptr;
}

void QmitkDataStorageSelectionConnector::SetAsSelectionProvider(QmitkDataNodeSelectionProvider* selectionProvider)
{
  m_SelectionProvider = selectionProvider;
  connect(m_DataStorageAbstractView, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), SLOT(FireGlobalSelectionChanged(QList<mitk::DataNode::Pointer>)));
}

void QmitkDataStorageSelectionConnector::RemoveAsSelectionProvider()
{
  disconnect(m_DataStorageAbstractView, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), this, SLOT(FireGlobalSelectionChanged(QList<mitk::DataNode::Pointer>)));
}

void QmitkDataStorageSelectionConnector::GlobalSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcePart, const berry::ISelection::ConstPointer& selection)
{
  if (sourcePart.IsNull())
  {
    return;
  }

  QList<mitk::DataNode::Pointer> nodes;
  if (selection.IsNull())
  {
    // propagate an empty list
    nodes = QList<mitk::DataNode::Pointer>();
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

  // set new (possibly empty) selection in the given data storage view
  m_DataStorageAbstractView->SetCurrentSelection(nodes);
}

void QmitkDataStorageSelectionConnector::FireGlobalSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
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
    for(const auto& node : nodes)
    {
      m_DataNodeItemModel->AddDataNode(node);
    }

    m_DataNodeSelectionModel->select(QItemSelection(m_DataNodeItemModel->index(0, 0), m_DataNodeItemModel->index(nodes.size() - 1, 0)), QItemSelectionModel::ClearAndSelect);
  }
}