/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <QmitkAbstractDataNodeAction.h>

#include "mitkIRenderWindowPart.h"

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// berry
#include <berryIWorkbenchPage.h>

QmitkAbstractDataNodeAction::QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer workbenchPartSite)
{
  m_WorkbenchPartSite = workbenchPartSite;
}

QmitkAbstractDataNodeAction::QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite* workbenchPartSite)
{
  m_WorkbenchPartSite = berry::IWorkbenchPartSite::Pointer(workbenchPartSite);
}

QmitkAbstractDataNodeAction::~QmitkAbstractDataNodeAction()
{
  // nothing here
}

void QmitkAbstractDataNodeAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }
}

QList<mitk::DataNode::Pointer> QmitkAbstractDataNodeAction::GetSelectedNodes()
{
  QList<mitk::DataNode::Pointer> selectedNodes;
  if (m_WorkbenchPartSite.Expired())
  {
    return selectedNodes;
  }

  berry::ISelection::ConstPointer selection = m_WorkbenchPartSite.Lock()->GetWorkbenchWindow()->GetSelectionService()->GetSelection();
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();

  if (currentSelection.IsNull() || currentSelection->IsEmpty())
  {
    return selectedNodes;
  }

  selectedNodes = QList<mitk::DataNode::Pointer>::fromStdList(currentSelection->GetSelectedDataNodes());
  return selectedNodes;
}

mitk::DataNode::Pointer QmitkAbstractDataNodeAction::GetSelectedNode()
{
  QList<mitk::DataNode::Pointer> selectedNodes = GetSelectedNodes();
  if (selectedNodes.empty())
  {
    return nullptr;
  }

  // no batch action; should only be called with a single node
  mitk::DataNode::Pointer dataNode = selectedNodes.front();
  if (nullptr == dataNode)
  {
    return nullptr;
  }

  return dataNode;
}
