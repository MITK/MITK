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

#include <QmitkDataNodeToggleVisibilityAction.h>
#include <QmitkDataNodeGlobalReinitAction.h>

// mitk core
#include <mitkRenderingManager.h>

QmitkDataNodeToggleVisibilityAction::QmitkDataNodeToggleVisibilityAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Toggle visibility"));
  InitializeAction();
}

QmitkDataNodeToggleVisibilityAction::QmitkDataNodeToggleVisibilityAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Toggle visibility"));
  InitializeAction();
}

QmitkDataNodeToggleVisibilityAction::~QmitkDataNodeToggleVisibilityAction()
{
  // nothing here
}

void QmitkDataNodeToggleVisibilityAction::InitializeAction()
{
  connect(this, &QmitkDataNodeToggleVisibilityAction::triggered, this, &QmitkDataNodeToggleVisibilityAction::OnActionTriggered);
}

void QmitkDataNodeToggleVisibilityAction::OnActionTriggered(bool checked)
{
  auto selectedNodes = GetSelectedNodes();
  bool isVisible;
  for (auto node : selectedNodes)
  {
    if (node.IsNotNull())
    {
      isVisible = false;
      node->GetBoolProperty("visible", isVisible);
      node->SetVisibility(!isVisible);
    }
  }

  if (false)//m_GlobalReinitOnNodeVisibilityChanged)
  {
    GlobalReinitAction::Run(m_WorkbenchPartSite.Lock(), m_DataStorage.Lock());
  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
