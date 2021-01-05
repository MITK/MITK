/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCorrectorTool2D.h"
#include "mitkCorrectorAlgorithm.h"

#include "mitkAbstractTransformGeometry.h"
#include "mitkImageReadAccessor.h"
#include "mitkToolManager.h"

#include "mitkCorrectorTool2D.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, CorrectorTool2D, "Correction tool");
}

mitk::CorrectorTool2D::CorrectorTool2D(int paintingPixelValue)
  : FeedbackContourTool("PressMoveRelease"), m_PaintingPixelValue(paintingPixelValue)
{
}

mitk::CorrectorTool2D::~CorrectorTool2D()
{
}

void mitk::CorrectorTool2D::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION("Move", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
}

const char **mitk::CorrectorTool2D::GetXPM() const
{
  return mitkCorrectorTool2D_xpm;
}

us::ModuleResource mitk::CorrectorTool2D::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Correction_48x48.png");
  return resource;
}

us::ModuleResource mitk::CorrectorTool2D::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Correction_Cursor_32x32.png");
  return resource;
}

const char *mitk::CorrectorTool2D::GetName() const
{
  return "Correction";
}

void mitk::CorrectorTool2D::Activated()
{
  Superclass::Activated();
}

void mitk::CorrectorTool2D::Deactivated()
{
  Superclass::Deactivated();
}

void mitk::CorrectorTool2D::OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  this->ClearsCurrentFeedbackContour(false);
  mitk::Point3D point = positionEvent->GetPositionInWorld();
  this->AddVertexToCurrentFeedbackContour(point);

  FeedbackContourTool::SetFeedbackContourVisible(true);
}

void mitk::CorrectorTool2D::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  mitk::Point3D point = positionEvent->GetPositionInWorld();
  this->AddVertexToCurrentFeedbackContour(point);

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::CorrectorTool2D::OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's
  // working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);

  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  // const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent)
    return;

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  if (!workingNode)
    return;

  auto *workingImage = dynamic_cast<Image *>(workingNode->GetData());
  const PlaneGeometry *planeGeometry((positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (!workingImage || !planeGeometry)
    return;

  const auto *abstractTransformGeometry(
    dynamic_cast<const AbstractTransformGeometry *>(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (!workingImage || abstractTransformGeometry)
    return;

  // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
  m_WorkingSlice = FeedbackContourTool::GetAffectedImageSliceAs2DImage(positionEvent, workingImage);

  if (m_WorkingSlice.IsNull())
  {
    MITK_ERROR << "Unable to extract slice." << std::endl;
    return;
  }

  auto feedbackContour = this->GetFeedbackContour();
  auto timestep = positionEvent->GetSender()->GetTimeStep(feedbackContour);
  mitk::ContourModel::Pointer singleTimestepContour = mitk::ContourModel::New();

  auto it = feedbackContour->Begin(timestep);
  auto end = feedbackContour->End(timestep);

  while (it != end)
  {
    singleTimestepContour->AddVertex((*it)->Coordinates);
    it++;
  }

  CorrectorAlgorithm::Pointer algorithm = CorrectorAlgorithm::New();
  algorithm->SetInput(m_WorkingSlice);
  algorithm->SetContour(singleTimestepContour);

  int activePixelValue = ContourModelUtils::GetActivePixelValue(workingImage);
  algorithm->SetFillColor(activePixelValue);

  try
  {
    algorithm->UpdateLargestPossibleRegion();
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Caught exception '" << e.what() << "'" << std::endl;
  }

  mitk::Image::Pointer resultSlice = FeedbackContourTool::GetAffectedImageSliceAs2DImage(positionEvent, workingImage);
  SegTool2D::WritePreviewOnWorkingImage(resultSlice, algorithm->GetOutput(), workingImage, activePixelValue);
  SegTool2D::WriteBackSegmentationResult(positionEvent, resultSlice);
}
