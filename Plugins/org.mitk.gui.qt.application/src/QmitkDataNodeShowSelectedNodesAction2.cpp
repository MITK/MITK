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

#include <QmitkDataNodeShowSelectedNodesAction.h>

// mitk core
#include <mitkRenderingManager.h>

QmitkDataNodeShowSelectedNodesAction::QmitkDataNodeShowSelectedNodesAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Show only selected nodes"));
  InitializeAction();
}

QmitkDataNodeShowSelectedNodesAction::QmitkDataNodeShowSelectedNodesAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Show only selected nodes"));
  InitializeAction();
}

QmitkDataNodeShowSelectedNodesAction::~QmitkDataNodeShowSelectedNodesAction()
{
  // nothing here
}

void QmitkDataNodeShowSelectedNodesAction::InitializeAction()
{
  connect(this, &QmitkDataNodeShowSelectedNodesAction::triggered, this, &QmitkDataNodeShowSelectedNodesAction::OnActionTriggered);
}

void QmitkDataNodeShowSelectedNodesAction::OnActionTriggered(bool /*checked*/)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  auto selectedNodes = GetSelectedNodes();
  auto nodeset = dataStorage->GetAll();
  for (auto& node : *nodeset)
  {
    if (node.IsNotNull())
    {
      node->SetVisibility(selectedNodes.contains(node));
      if(node->GetData() == nullptr)
      {
        node->SetVisibility(true);
        mitk::DataStorage::SetOfObjects::ConstPointer derivations = dataStorage->GetDerivations(node);
        for (mitk::DataStorage::SetOfObjects::const_iterator iter = derivations->begin(); iter != derivations->end(); ++iter)
        {
          (*iter)->SetVisibility(true);
        }
      }
    }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
