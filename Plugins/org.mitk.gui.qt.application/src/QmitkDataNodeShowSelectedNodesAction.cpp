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
#include <mitkNodePredicateFirstLevel.h>

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

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  auto dataNodes = GetSelectedNodes();
  mitk::NodePredicateFirstLevel::Pointer nodeFirstLevelFilterPredicate = mitk::NodePredicateFirstLevel::New(dataStorage);
  auto nodesetFirstLevel = dataStorage->GetSubset(nodeFirstLevelFilterPredicate);

  for (auto& node : *nodesetFirstLevel)
  {
    if (node.IsNotNull() && node->GetData() != nullptr)
    {
      node->SetVisibility(dataNodes.contains(node), baseRenderer);
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
