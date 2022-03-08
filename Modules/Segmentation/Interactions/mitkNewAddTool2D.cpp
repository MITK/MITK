/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//#include <mitkContourModelUtils.h>
#include <mitkNewAddTool2D.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

#include <type_traits>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, NewAddTool2D, "New Add tool");
}

mitk::NewAddTool2D::NewAddTool2D() : EditableContourTool("LiveWireTool") 
{
}

mitk::NewAddTool2D::~NewAddTool2D()
{}

void mitk::NewAddTool2D::ConnectActionsAndFunctions()
{
  mitk::EditableContourTool::ConnectActionsAndFunctions();
  CONNECT_FUNCTION("MovePoint", OnMouseMoved);
}

void mitk::NewAddTool2D::ReleaseInteractors()
{
  this->EnableContourInteraction(false);
  m_ContourInteractors.clear();
}

const char **mitk::NewAddTool2D::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::NewAddTool2D::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("NewAdd_48x48.png");
}

us::ModuleResource mitk::NewAddTool2D::GetCursorIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("NewAdd_Cursor_32x32.png");
}

const char *mitk::NewAddTool2D::GetName() const
{
  return "New Add";
}

void mitk::NewAddTool2D::EnableContourInteraction(bool on)
{
  for (const auto &interactor : m_ContourInteractors)
    interactor->EnableInteraction(on);
}

void mitk::NewAddTool2D::OnInitContour(StateMachineAction *s, InteractionEvent *interactionEvent) 
{
  mitk::EditableContourTool::OnInitContour(s, interactionEvent);

  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (nullptr == positionEvent)
    return;

  auto workingDataNode = this->GetWorkingDataNode();

  if (!IsPositionEventInsideImageRegion(positionEvent, workingDataNode->GetData()))
  {
    this->ResetToStartState();
    return;
  }

  // Map click to pixel coordinates
  auto click = positionEvent->GetPositionInWorld();

  // Set initial start point
  m_Contour->AddVertex(click, true);
  m_PreviewContour->AddVertex(click, false);
  m_ClosureContour->AddVertex(click);

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::NewAddTool2D::OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent) 
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (nullptr == positionEvent)
    return;

  if (m_PlaneGeometry.IsNotNull())
  {
    // Check if the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::sqrteps)
      return;
  }

  m_Contour->AddVertex(positionEvent->GetPositionInWorld(), true);

  std::for_each(m_PreviewContour->IteratorBegin(), m_PreviewContour->IteratorEnd(), [this](ContourElement::VertexType* vertex) 
  {
    m_PreviewContour->RemoveVertex(vertex);
  });

  m_PreviewContour->AddVertex(positionEvent->GetPositionInWorld());
  m_PreviewContour->Update();

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::NewAddTool2D::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent) 
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (nullptr == positionEvent)
    return;

  if (m_PlaneGeometry.IsNotNull())
  {
    // Check if the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::sqrteps)
      return;
  }

  if (m_PreviewContour->GetNumberOfVertices() > 1)
  {
    m_PreviewContour->RemoveVertexAt(m_PreviewContour->GetNumberOfVertices() - 1);
  }
  m_PreviewContour->AddVertex(positionEvent->GetPositionInWorld());
  m_PreviewContour->Update();

  this->UpdateClosureContour(positionEvent->GetPositionInWorld());

  RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::NewAddTool2D::OnFinish(StateMachineAction *s, InteractionEvent *interactionEvent) 
{
  mitk::EditableContourTool::OnFinish(s, interactionEvent);
  this->FinishTool();
}

void mitk::NewAddTool2D::FinishTool() 
{
  mitk::EditableContourTool::FinishTool();

  m_ContourInteractor = mitk::ContourModelInteractor::New();
  m_ContourInteractor->SetDataNode(m_ContourNode);
  m_ContourInteractor->LoadStateMachine("ContourModelModificationInteractor.xml", us::GetModuleContext()->GetModule());
  m_ContourInteractor->SetEventConfig("ContourModelModificationConfig.xml", us::GetModuleContext()->GetModule());

  m_ContourNode->SetDataInteractor(m_ContourInteractor.GetPointer());

  this->m_ContourInteractors.push_back(m_ContourInteractor);
}
