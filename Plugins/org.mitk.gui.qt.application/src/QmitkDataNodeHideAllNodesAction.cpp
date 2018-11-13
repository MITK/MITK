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

#include <QmitkDataNodeHideAllNodesAction.h>

// mitk core
#include <mitkRenderingManager.h>

QmitkDataNodeHideAllNodesAction::QmitkDataNodeHideAllNodesAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Hide all nodes"));
  InitializeAction();
}

QmitkDataNodeHideAllNodesAction::QmitkDataNodeHideAllNodesAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Hide all nodes"));
  InitializeAction();
}

QmitkDataNodeHideAllNodesAction::~QmitkDataNodeHideAllNodesAction()
{
  // nothing here
}

void QmitkDataNodeHideAllNodesAction::InitializeAction()
{
  connect(this, &QmitkDataNodeHideAllNodesAction::triggered, this, &QmitkDataNodeHideAllNodesAction::OnActionTriggered);
}

void QmitkDataNodeHideAllNodesAction::OnActionTriggered(bool checked)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  mitk::BaseRenderer* baseRenderer;
  if (m_BaseRenderer.IsExpired())
  {
    baseRenderer = nullptr;
  }
  else
  {
    baseRenderer = m_BaseRenderer.Lock();
  }

  auto nodeset = m_DataStorage.Lock()->GetAll();
  for (auto& node : *nodeset)
  {
    if (node.IsNotNull())
    {
      node->SetVisibility(false, baseRenderer);
    }
  }

  if (nullptr == baseRenderer)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
  }
}
