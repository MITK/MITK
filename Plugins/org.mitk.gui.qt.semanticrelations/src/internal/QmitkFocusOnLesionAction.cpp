/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkFocusOnLesionAction.h"
#include "QmitkLabelSetJumpToAction.h"

// mitk core
#include <mitkImage.h>
#include <mitkRenderingManager.h>

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowLayerUtilities.h>

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationsInference.h>

// qt
#include <QMessageBox>
#include <QMenu>

QmitkFocusOnLesionAction::QmitkFocusOnLesionAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Focus on lesion"));
  InitializeAction();
}

QmitkFocusOnLesionAction::QmitkFocusOnLesionAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Focus on lesion"));
  InitializeAction();
}

void QmitkFocusOnLesionAction::SetSelectedLesion(mitk::SemanticTypes::Lesion selectedLesion)
{
  m_Lesion = selectedLesion;
}

void QmitkFocusOnLesionAction::InitializeAction()
{
  setCheckable(true);
  connect(this, &QAction::triggered, this, &QmitkFocusOnLesionAction::OnActionTriggered);

  SetControlledRenderer();
}

void QmitkFocusOnLesionAction::OnActionTriggered(bool /*checked*/)
{
  if (m_WorkbenchPartSite.Expired())
  {
    return;
  }

  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();
  auto renderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  renderWindowLayerController->SetDataStorage(dataStorage);
  // check each renderer to see if it contains a segmentation represented by the currently selected lesion
  for (const auto& renderer : m_ControlledRenderer)
  {
    // get currently fixed layer nodes of the specified renderer
    auto layerStack = mitk::RenderWindowLayerUtilities::GetLayerStack(dataStorage, renderer, true);
    // check each node to see if it is a segmentation represented by the currently selected lesion
    for (const auto& layer : layerStack)
    {
      auto dataNode = layer.second;
      bool isVisible = false;
      if (dataNode.IsNotNull())
      {
        dataNode->GetBoolProperty("visible", isVisible, renderer);
      }

      // only focus in renderer where the segmentation is visible
      if (!isVisible)
      {
        continue;
      }

      auto currentLesion = mitk::SemanticRelationsInference::GetLesionOfSegmentation(dataNode);
      if (currentLesion.UID == m_Lesion.UID)
      {
        // jump to this lesion in the specified renderer
        LabelSetJumpToAction::Run(m_WorkbenchPartSite.Lock(), dataNode, renderer);
      }
    }
  }
}

void QmitkFocusOnLesionAction::SetControlledRenderer()
{
  const mitk::RenderingManager::RenderWindowVector allRegisteredRenderWindows =
    mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  mitk::BaseRenderer *baseRenderer = nullptr;
  m_ControlledRenderer.clear();
  for (const auto& renderWindow : allRegisteredRenderWindows)
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow);
    if (nullptr != baseRenderer)
    {
      m_ControlledRenderer.push_back(baseRenderer);
    }
  }
}
