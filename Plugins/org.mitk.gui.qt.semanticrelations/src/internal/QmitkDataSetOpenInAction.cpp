/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkDataSetOpenInAction.h"

// mitk core
#include <mitkImage.h>
#include <mitkRenderingManager.h>

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowLayerUtilities.h>

// semantic relations module
#include "mitkNodePredicates.h"

// mitk gui qt application plugin
#include <QmitkDataNodeReinitAction.h>

// qt
#include <QMessageBox>
#include <QMenu>

namespace OpenInAction
{
  void Run(mitk::DataStorage::Pointer dataStorage, mitk::DataNode::Pointer imageNode, mitk::BaseRenderer* renderer /*= nullptr*/)
  {
    if (dataStorage.IsNull())
    {
      return;
    }

    if (imageNode.IsNull())
    {
      return;
    }

    auto renderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
    renderWindowLayerController->SetDataStorage(dataStorage);

    // get currently fixed layer nodes of the specified renderer
    // remove them from the specified renderer
    auto layerStack = mitk::RenderWindowLayerUtilities::GetLayerStack(dataStorage, renderer, true);
    for (auto& fixedLayer : layerStack)
    {
      // hide all nodes of the specified renderer
      fixedLayer.second->SetVisibility(false, renderer);
      fixedLayer.second->Modified();
    }

    // add the selected data node to the specified renderer
    // only needed in case the render window manager plugin is not used
    imageNode->SetBoolProperty("fixedLayer", true, renderer);
    // make is visible
    imageNode->SetVisibility(true, renderer);
    imageNode->Modified();
    // move node to front, which also request a render update
    renderWindowLayerController->MoveNodeToFront(imageNode, renderer);

    QList<mitk::DataNode::Pointer> visibleNodes;
    visibleNodes.push_back(imageNode);
    // get all corresponding segmentations of this node
    mitk::DataStorage::SetOfObjects::ConstPointer segmentationNodes = dataStorage->GetDerivations(imageNode, mitk::NodePredicates::GetSegmentationPredicate(), false);
    for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
    {
      auto segmentation = it->Value();
      segmentation->SetBoolProperty("fixedLayer", true, renderer);
      segmentation->SetVisibility(true, renderer);
      // move node to front, which also request a render update
      renderWindowLayerController->MoveNodeToFront(segmentation, renderer);
      visibleNodes.push_back(segmentation);
    }

    ReinitAction::Run(berry::IWorkbenchPartSite::Pointer(), dataStorage, visibleNodes, renderer);
  }
}

QmitkDataSetOpenInAction::QmitkDataSetOpenInAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QmitkDataNodeOpenInAction(parent, workbenchPartSite)
{
  setText(tr("Open in"));
  InitializeAction();
}

QmitkDataSetOpenInAction::QmitkDataSetOpenInAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QmitkDataNodeOpenInAction(parent, berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Open in"));
  InitializeAction();
}

void QmitkDataSetOpenInAction::InitializeAction()
{
  setCheckable(true);

  setMenu(new QMenu);
  connect(menu(), &QMenu::aboutToShow, this, &QmitkDataSetOpenInAction::OnMenuAboutToShow);

  SetControlledRenderer();
}

void QmitkDataSetOpenInAction::OnMenuAboutToShow()
{
  menu()->clear();
  QAction* action;

  QStringList rendererNames;
  for (const auto& renderer : m_ControlledRenderer)
  {
    rendererNames.append(renderer->GetName());
  }

  rendererNames.sort();
  for (const auto& rendererName : rendererNames)
  {
    action = menu()->addAction(rendererName);
    connect(action, &QAction::triggered, this, &QmitkDataSetOpenInAction::OnActionTriggered);
  }
}

void QmitkDataSetOpenInAction::OnActionTriggered(bool /*checked*/)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if (nullptr == senderAction)
  {
    return;
  }

  std::string selectedRenderer = senderAction->text().toStdString();
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetByName(selectedRenderer);
  if (nullptr == renderer)
  {
    return;
  }

  OpenInAction::Run(m_DataStorage.Lock(), dataNode, renderer);
}
