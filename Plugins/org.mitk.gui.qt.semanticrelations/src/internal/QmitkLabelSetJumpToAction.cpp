/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk gui qt application plugin
#include "QmitkLabelSetJumpToAction.h"

// mitk core
#include <mitkImage.h>
#include <mitkRenderingManager.h>
#include <mitkWorkbenchUtil.h>

// mitk multi label module
#include <mitkLabelSetImage.h>

// qt
#include <QMessageBox>
#include <QMenu>

namespace LabelSetJumpToAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite, const mitk::DataNode* dataNode, mitk::BaseRenderer* baseRenderer /*= nullptr*/)
  {
    if (workbenchPartSite.IsNull())
    {
      return;
    }

    if (nullptr == dataNode)
    {
      return;
    }

    mitk::LabelSetImage* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());
    if (nullptr == labelSetImage)
    {
      return;
    }

    unsigned int activeLayer = labelSetImage->GetActiveLayer();
    mitk::Label* activeLabel = labelSetImage->GetActiveLabel(activeLayer);
    labelSetImage->UpdateCenterOfMass(activeLabel->GetValue(), activeLayer);
    const mitk::Point3D& centerPosition = activeLabel->GetCenterOfMassCoordinates();
    if (centerPosition.GetVnlVector().max_value() > 0.0)
    {
      auto renderWindowPart = mitk::WorkbenchUtil::GetRenderWindowPart(workbenchPartSite->GetPage(), mitk::WorkbenchUtil::NONE);
      if (nullptr == renderWindowPart)
      {
        renderWindowPart = mitk::WorkbenchUtil::OpenRenderWindowPart(workbenchPartSite->GetPage(), false);
        if (nullptr == renderWindowPart)
        {
          // no render window available
          return;
        }
      }

      auto segmentation = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());
      if (nullptr != segmentation)
      {
        if (nullptr == baseRenderer)
        {
          renderWindowPart->SetSelectedPosition(centerPosition);
          mitk::RenderingManager::GetInstance()->InitializeViews(segmentation->GetTimeGeometry());
        }
        else
        {
          renderWindowPart->SetSelectedPosition(centerPosition, baseRenderer->GetName());
          mitk::RenderingManager::GetInstance()->InitializeView(baseRenderer->GetRenderWindow(), segmentation->GetTimeGeometry());
        }
      }
    }
  }
}

QmitkLabelSetJumpToAction::QmitkLabelSetJumpToAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  setText(tr("Jump to"));
  InitializeAction();
}

QmitkLabelSetJumpToAction::QmitkLabelSetJumpToAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  setText(tr("Jump to"));
  InitializeAction();
}

void QmitkLabelSetJumpToAction::InitializeAction()
{
  connect(this, &QAction::triggered, this, &QmitkLabelSetJumpToAction::OnActionTriggered);
}

void QmitkLabelSetJumpToAction::OnActionTriggered(bool /*checked*/)
{
  if (m_WorkbenchPartSite.Expired())
  {
    return;
  }

  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();
  LabelSetJumpToAction::Run(m_WorkbenchPartSite.Lock(), dataNode, baseRenderer);
}
