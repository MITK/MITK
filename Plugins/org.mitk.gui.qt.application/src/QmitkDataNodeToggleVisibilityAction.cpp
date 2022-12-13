/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataNodeToggleVisibilityAction.h>
#include <QmitkDataNodeGlobalReinitAction.h>

// mitk core
#include <mitkRenderingManager.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

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

    auto* prefService = mitk::CoreServices::GetPreferencesService();

    auto* preferences = prefService->GetSystemPreferences()->Node(QmitkDataNodeGlobalReinitAction::ACTION_ID.toStdString());
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
  auto workbenchPartSite = m_WorkbenchPartSite.Lock();

  if (workbenchPartSite.IsNull())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  auto dataNodes = GetSelectedNodes();
  ToggleVisibilityAction::Run(workbenchPartSite, dataStorage, dataNodes, baseRenderer);
}
