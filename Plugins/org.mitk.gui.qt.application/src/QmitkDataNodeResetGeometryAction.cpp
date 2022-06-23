/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataNodeResetGeometryAction.h>

// mitk core
#include <mitkImage.h>
#include <mitkRenderingManager.h>

// mitk gui common plugin
#include <mitkWorkbenchUtil.h>

// namespace that contains the concrete action
namespace ResetGeometryAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite, const mitk::TimeGeometry* referenceGeometry, mitk::BaseRenderer* baseRenderer /*= nullptr*/)
  {

    if (workbenchPartSite.IsNull())
    {
      return;
    }

    auto* renderWindowPart = mitk::WorkbenchUtil::GetRenderWindowPart(workbenchPartSite->GetPage(), mitk::WorkbenchUtil::NONE);
    if (nullptr == renderWindowPart)
    {
      renderWindowPart = mitk::WorkbenchUtil::OpenRenderWindowPart(workbenchPartSite->GetPage(), false);
      if (nullptr == renderWindowPart)
      {
        // no render window available
        return;
      }
    }

    if (nullptr == referenceGeometry)
    {
      return;
    }

    mitk::TimeStepType imageTimeStep = 0;

    // store the current position to set it again later, if the camera should not be reset
    mitk::Point3D currentPosition = renderWindowPart->GetSelectedPosition();

    // store the current time step to set it again later, if the camera should not be reset
    auto* renderingManager = mitk::RenderingManager::GetInstance();
    const auto currentTimePoint = renderingManager->GetTimeNavigationController()->GetSelectedTimePoint();
    if (referenceGeometry->IsValidTimePoint(currentTimePoint))
    {
      imageTimeStep = referenceGeometry->TimePointToTimeStep(currentTimePoint);
    }

    if (nullptr == baseRenderer)
    {
      renderingManager->InitializeViews(referenceGeometry, mitk::RenderingManager::REQUEST_UPDATE_ALL, false);
    }
    else
    {
      renderingManager->InitializeView(baseRenderer->GetRenderWindow(), referenceGeometry, false);
    }

    renderWindowPart->SetSelectedPosition(currentPosition);
    renderingManager->GetTimeNavigationController()->GetTime()->SetPos(imageTimeStep);
  }
} // namespace ResetGeometryAction

QmitkDataNodeResetGeometryAction::QmitkDataNodeResetGeometryAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  this->setText(tr("Reset geometry"));
  this->InitializeAction();
}

QmitkDataNodeResetGeometryAction::QmitkDataNodeResetGeometryAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QmitkDataNodeResetGeometryAction(parent, berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
}

void QmitkDataNodeResetGeometryAction::InitializeAction()
{
  connect(this, &QmitkDataNodeResetGeometryAction::triggered, this, &QmitkDataNodeResetGeometryAction::OnActionTriggered);
}

void QmitkDataNodeResetGeometryAction::OnActionTriggered(bool /*checked*/)
{
  auto workbenchPartSite = m_WorkbenchPartSite.Lock();

  if (workbenchPartSite.IsNull())
  {
    return;
  }

  auto baseRenderer = this->GetBaseRenderer();

  auto selectedNode = this->GetSelectedNode();
  mitk::Image::ConstPointer selectedImage = dynamic_cast<mitk::Image*>(selectedNode->GetData());
  if (selectedImage.IsNull())
  {
    return;
  }

  ResetGeometryAction::Run(workbenchPartSite, selectedImage->GetTimeGeometry(), baseRenderer);
}
