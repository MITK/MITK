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

// semantic relations plugin
#include "QmitkDataNodeSetControlPointAction.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// semantic relations UI module
#include "QmitkControlPointDialog.h"

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// berry
#include <berryISelectionService.h>
#include <berryIWorkbenchPage.h>

// qt
#include <QInputDialog>

QmitkDataNodeSetControlPointAction::QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , m_WorkbenchPartSite(workbenchPartSite)
{
  setText(tr("Set control point"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetControlPointAction::QmitkDataNodeSetControlPointAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , m_WorkbenchPartSite(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Set control point"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeSetControlPointAction::~QmitkDataNodeSetControlPointAction()
{
  // nothing here
}

void QmitkDataNodeSetControlPointAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
    m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(m_DataStorage.Lock());
  }
}

void QmitkDataNodeSetControlPointAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkDataNodeSetControlPointAction::OnActionTriggered);
}

void QmitkDataNodeSetControlPointAction::OnActionTriggered(bool checked)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  QmitkControlPointDialog* inputDialog = new QmitkControlPointDialog(m_Parent);
  inputDialog->setWindowTitle("Set control point");
  inputDialog->SetCurrentDate(mitk::GetDICOMDateFromDataNode(dataNode));

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  const QDate& userSelectedDate = inputDialog->GetCurrentDate();
  mitk::SemanticTypes::ControlPoint controlPoint;
  controlPoint.UID = mitk::UIDGeneratorBoost::GenerateUID();
  controlPoint.year = userSelectedDate.year();
  controlPoint.month = userSelectedDate.month();
  controlPoint.day = userSelectedDate.day();

  try
  {
    m_SemanticRelations->SetControlPointOfData(dataNode, controlPoint);
  }
  catch (const mitk::SemanticRelationException&)
  {
    return;
  }
}

QList<mitk::DataNode::Pointer> QmitkDataNodeSetControlPointAction::GetSelectedNodes()
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

mitk::DataNode::Pointer QmitkDataNodeSetControlPointAction::GetSelectedNode()
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
