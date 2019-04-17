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

// berry
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

// namespace that contains the concrete action
namespace ToggleVisibilityAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite, mitk::DataStorage::Pointer dataStorage, const QList<mitk::DataNode::Pointer>& selectedNodes /*= QList<mitk::DataNode::Pointer>()*/, mitk::BaseRenderer* baseRenderer /*= nullptr*/)
  {
    bool isVisible;
    for (auto& node : selectedNodes)
    {
      if (node.IsNotNull())
      {
        isVisible = false;
        node->GetBoolProperty("visible", isVisible, baseRenderer);
        node->SetVisibility(!isVisible, baseRenderer);
      }
    }

    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

    berry::IPreferences::Pointer preferences = prefService->GetSystemPreferences()->Node(QmitkDataNodeGlobalReinitAction::ACTION_ID);
    bool globalReinit = preferences->GetBool("Call global reinit if node visibility is changed", false);
    if (globalReinit)
    {
      GlobalReinitAction::Run(workbenchPartSite, dataStorage);
    }
    else
    {
      if (nullptr == baseRenderer)
      {
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
      else
      {
        mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
      }
    }
  }
}

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

void QmitkDataNodeToggleVisibilityAction::InitializeAction()
{
  connect(this, &QmitkDataNodeToggleVisibilityAction::triggered, this, &QmitkDataNodeToggleVisibilityAction::OnActionTriggered);
}

void QmitkDataNodeToggleVisibilityAction::OnActionTriggered(bool /*checked*/)
{
  if (m_WorkbenchPartSite.Expired())
  {
    return;
  }

  if (m_DataStorage.IsExpired())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  auto dataNodes = GetSelectedNodes();
  ToggleVisibilityAction::Run(m_WorkbenchPartSite.Lock(), m_DataStorage.Lock(), dataNodes, baseRenderer);
}
