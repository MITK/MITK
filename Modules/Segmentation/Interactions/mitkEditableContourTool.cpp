/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEditableContourTool.h"
#include <mitkToolManager.h>


mitk::EditableContourTool::EditableContourTool() : FeedbackContourTool("EditableContourTool"), m_AutoConfirm(true), m_AddMode(true)
{}

mitk::EditableContourTool::~EditableContourTool()
{
  this->ReleaseHelperObjects();
  this->ReleaseInteractors();
}


void mitk::EditableContourTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("InitObject", OnInitContour);
  CONNECT_FUNCTION("AddPoint", OnAddPoint);
  CONNECT_FUNCTION("CtrlAddPoint", OnAddPoint);
  CONNECT_FUNCTION("Drawing", OnDrawing);
  CONNECT_FUNCTION("EndDrawing", OnEndDrawing);
  CONNECT_FUNCTION("FinishContour", OnFinish);
  CONNECT_FUNCTION("CtrlMovePoint", OnMouseMoved);
}

void mitk::EditableContourTool::Activated()
{
  Superclass::Activated();
  this->ResetToStartState();
  this->EnableContourInteraction(true);
}

void mitk::EditableContourTool::Deactivated()
{
  this->ClearContour();
  Superclass::Deactivated();
}

void mitk::EditableContourTool::ConfirmSegmentation(bool resetStatMachine)
{
  auto referenceImage = this->GetReferenceData();
  auto workingImage = this->GetWorkingData();

  if (nullptr != referenceImage && nullptr != workingImage)
  {
    std::vector<SliceInformation> sliceInfos;

    const auto currentTimePoint =
      mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    TimeStepType workingImageTimeStep = workingImage->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);

    auto contour = this->GetContour();
    if (nullptr == contour || contour->IsEmpty())
      return;

    auto workingSlice =
      this->GetAffectedImageSliceAs2DImage(m_PlaneGeometry, workingImage, workingImageTimeStep)->Clone();
    sliceInfos.emplace_back(workingSlice, m_PlaneGeometry, workingImageTimeStep);

    auto projectedContour = ContourModelUtils::ProjectContourTo2DSlice(workingSlice, contour);
    int activePixelValue = ContourModelUtils::GetActivePixelValue(workingImage);
    if (!m_AddMode)
    {
      activePixelValue = 0;
    }

    ContourModelUtils::FillContourInSlice(projectedContour, workingSlice, workingImage, activePixelValue);

    this->WriteBackSegmentationResults(sliceInfos);
  }

  this->ReleaseHelperObjects();
  this->ReleaseInteractors();
  if (resetStatMachine) this->ResetToStartState();
}

void mitk::EditableContourTool::ClearContour()
{
  this->ReleaseHelperObjects();
  this->ReleaseInteractors();
  this->ResetToStartState();
}

bool mitk::EditableContourTool::IsEditingContour() const
{
  return (nullptr != GetContour()) && !this->IsDrawingContour();
};


bool mitk::EditableContourTool::IsDrawingContour() const
{
  return m_PreviewContour.IsNotNull();
};

mitk::Point3D mitk::EditableContourTool::PrepareInitContour(const Point3D& clickedPoint)
{ //default implementation does nothing
  return clickedPoint;
}

void mitk::EditableContourTool::OnInitContour(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (nullptr == positionEvent)
    return;

  auto workingDataNode = this->GetWorkingDataNode();

  if (nullptr != this->GetContour())
  {
    this->ConfirmSegmentation(false);
  }

  if (!IsPositionEventInsideImageRegion(positionEvent, workingDataNode->GetData()))
  {
    this->ResetToStartState();
    return;
  }

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  auto contour = this->CreateNewContour();
  m_ContourNode = mitk::DataNode::New();
  m_ContourNode->SetData(contour);
  m_ContourNode->SetName("working contour node");
  m_ContourNode->SetProperty("layer", IntProperty::New(100));
  m_ContourNode->AddProperty("fixedLayer", BoolProperty::New(true));
  m_ContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_ContourNode->AddProperty("contour.color", ColorProperty::New(1.0f, 1.0f, 0.0f), nullptr, true);
  m_ContourNode->AddProperty("contour.points.color", ColorProperty::New(1.0f, 0.0f, 0.1f), nullptr, true);
  m_ContourNode->AddProperty("contour.controlpoints.show", BoolProperty::New(true), nullptr, true);

  m_PreviewContour = this->CreateNewContour();
  m_PreviewContourNode = mitk::DataNode::New();
  m_PreviewContourNode->SetData(m_PreviewContour);
  m_PreviewContourNode->SetName("active preview node");
  m_PreviewContourNode->SetProperty("layer", IntProperty::New(101));
  m_PreviewContourNode->AddProperty("fixedLayer", BoolProperty::New(true));
  m_PreviewContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PreviewContourNode->AddProperty("contour.color", ColorProperty::New(0.1f, 1.0f, 0.1f), nullptr, true);
  m_PreviewContourNode->AddProperty("contour.width", mitk::FloatProperty::New(4.0f), nullptr, true);


  m_ClosureContour = this->CreateNewContour();
  m_ClosureContourNode = mitk::DataNode::New();
  m_ClosureContourNode->SetData(m_ClosureContour);
  m_ClosureContourNode->SetName("active closure node");
  m_ClosureContourNode->SetProperty("layer", IntProperty::New(101));
  m_ClosureContourNode->AddProperty("fixedLayer", BoolProperty::New(true));
  m_ClosureContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_ClosureContourNode->AddProperty("contour.color", ColorProperty::New(0.0f, 1.0f, 0.1f), nullptr, true);
  m_ClosureContourNode->AddProperty("contour.width", mitk::FloatProperty::New(2.0f), nullptr, true);

  m_CurrentRestrictedArea = this->CreateNewContour();

  auto dataStorage = this->GetToolManager()->GetDataStorage();
  dataStorage->Add(m_ContourNode, workingDataNode);
  dataStorage->Add(m_PreviewContourNode, workingDataNode);
  dataStorage->Add(m_ClosureContourNode, workingDataNode);

  m_ReferenceDataSlice = this->GetAffectedReferenceSlice(positionEvent);

  auto origin = m_ReferenceDataSlice->GetSlicedGeometry()->GetOrigin();
  m_ReferenceDataSlice->GetSlicedGeometry()->WorldToIndex(origin, origin);
  m_ReferenceDataSlice->GetSlicedGeometry()->IndexToWorld(origin, origin);
  m_ReferenceDataSlice->GetSlicedGeometry()->SetOrigin(origin);

  // Remember PlaneGeometry to determine if events were triggered in the same plane
  m_PlaneGeometry = interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry();

  // Map click to pixel coordinates
  auto click = positionEvent->GetPositionInWorld();

  click = this->PrepareInitContour(click);

  this->InitializePreviewContour(click);
  // Set initial start point
  contour->AddVertex(click, true);
  m_PreviewContour->AddVertex(click, false);
  m_ClosureContour->AddVertex(click);

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::EditableContourTool::FinalizePreviewContour(const Point3D& /*clickedPoint*/)
{ // Remove duplicate first vertex, it's already contained in m_ContourNode
  m_PreviewContour->RemoveVertexAt(0);

  m_PreviewContour->SetControlVertexAt(m_PreviewContour->GetNumberOfVertices() - 1);
}

void mitk::EditableContourTool::InitializePreviewContour(const Point3D& clickedPoint)
{ //default implementation only clears the preview and sets the start point
  m_PreviewContour = this->CreateNewContour();
  m_PreviewContour->AddVertex(clickedPoint);
  m_PreviewContourNode->SetData(m_PreviewContour);
}

void mitk::EditableContourTool::UpdatePreviewContour(const Point3D& clickedPoint)
{ //default implementation draws just a simple line to position
  if (m_PreviewContour->GetNumberOfVertices() > 2)
  {
    auto contour = this->GetContour();
    this->InitializePreviewContour(contour->GetVertexAt(contour->GetNumberOfVertices() - 1)->Coordinates);
  }

  if (m_PreviewContour->GetNumberOfVertices() == 2)
  {
    m_PreviewContour->RemoveVertexAt(m_PreviewContour->GetNumberOfVertices()-1);
  }

  m_PreviewContour->AddVertex(clickedPoint);
}

void mitk::EditableContourTool::OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);

  if (nullptr == positionEvent)
    return;

  if (m_PlaneGeometry.IsNotNull())
  {
    // Check if the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::sqrteps)
      return;
  }

  this->FinalizePreviewContour(positionEvent->GetPositionInWorld());

  // Merge contours
  auto contour = this->GetContour();
  contour->Concatenate(m_PreviewContour);

  this->InitializePreviewContour(positionEvent->GetPositionInWorld());

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::EditableContourTool::OnDrawing(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!positionEvent)
    return;

  m_PreviewContourNode->SetVisibility(false);

  auto contour = this->GetContour();
  contour->AddVertex(positionEvent->GetPositionInWorld(), false);
  UpdateClosureContour(positionEvent->GetPositionInWorld());

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::EditableContourTool::OnEndDrawing(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!positionEvent)
    return;

  auto contour = this->GetContour();
  auto controlVs = contour->GetControlVertices(0);
  if (!controlVs.empty())
  { //add the last control point (after that the draw part start)
    m_CurrentRestrictedArea->AddVertex(controlVs.back()->Coordinates);
  }
  m_PreviewContourNode->SetVisibility(true);
  contour->SetControlVertexAt(contour->GetNumberOfVertices() - 1);

  //add the just created/set last control point (with it the draw part ends)
  m_CurrentRestrictedArea->AddVertex(contour->GetVertexAt(contour->GetNumberOfVertices() - 1)->Coordinates);

  this->InitializePreviewContour(positionEvent->GetPositionInWorld());

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::EditableContourTool::OnMouseMoved(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);

  if (nullptr == positionEvent)
    return;

  if (m_PlaneGeometry.IsNotNull())
  {
    // Check if the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::sqrteps)
      return;
  }

  this->UpdatePreviewContour(positionEvent->GetPositionInWorld());
  this->UpdateClosureContour(positionEvent->GetPositionInWorld());

  RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::EditableContourTool::OnFinish(StateMachineAction *, InteractionEvent *interactionEvent)
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

  this->FinalizePreviewContour(positionEvent->GetPositionInWorld());

  this->FinishTool();

  // Merge contours
  auto contour = this->GetContour();
  contour->Concatenate(m_PreviewContour);

  auto numberOfTimesteps = static_cast<int>(contour->GetTimeSteps());

  for (int i = 0; i <= numberOfTimesteps; ++i)
    contour->Close(i);

  this->ReleaseHelperObjects(false);

  if (m_AutoConfirm)
  {
    this->ConfirmSegmentation();
  }
}

void mitk::EditableContourTool::ReleaseHelperObjects(bool includeWorkingContour)
{
  this->RemoveHelperObjectsFromDataStorage(includeWorkingContour);

  if (includeWorkingContour)
  {
    m_ContourNode = nullptr;
    m_CurrentRestrictedArea = nullptr;
  }

  m_PreviewContourNode = nullptr;
  m_PreviewContour = nullptr;

  m_ClosureContourNode = nullptr;
  m_ClosureContour = nullptr;
}

void mitk::EditableContourTool::RemoveHelperObjectsFromDataStorage(bool includeWorkingContour)
{
  auto dataStorage = this->GetToolManager()->GetDataStorage();

  if (nullptr == dataStorage)
    return;

  if (includeWorkingContour)
  {
    if (m_ContourNode.IsNotNull())
      dataStorage->Remove(m_ContourNode);
  }

  if (m_PreviewContourNode.IsNotNull())
    dataStorage->Remove(m_PreviewContourNode);

  if (m_ClosureContourNode.IsNotNull())
    dataStorage->Remove(m_ClosureContourNode);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::ContourModel::Pointer mitk::EditableContourTool::CreateNewContour() const
{
  auto workingData = this->GetWorkingData();
  if (nullptr == workingData)
  {
    this->InteractiveSegmentationBugMessage(
      "Cannot create new contour. No valid working data is set. Application is in invalid state.");
    mitkThrow() << "Cannot create new contour. No valid working data is set. Application is in invalid state.";
  }

  auto contour = ContourModel::New();

  // generate a time geometry that is always visible as the working contour should always be.
  auto contourTimeGeometry = ProportionalTimeGeometry::New();
  contourTimeGeometry->SetStepDuration(std::numeric_limits<TimePointType>::max());
  contourTimeGeometry->SetTimeStepGeometry(contour->GetTimeGeometry()->GetGeometryForTimeStep(0)->Clone(), 0);
  contour->SetTimeGeometry(contourTimeGeometry);

  return contour;
}

void mitk::EditableContourTool::UpdateClosureContour(mitk::Point3D endpoint) 
{
  if (m_ClosureContour->GetNumberOfVertices() > 2)
  {
    m_ClosureContour = this->CreateNewContour();
    m_ClosureContourNode->SetData(m_ClosureContour);
  }

  if (m_ClosureContour->GetNumberOfVertices() == 0)
  {
    auto contour = this->GetContour();
    m_ClosureContour->AddVertex(contour->GetVertexAt(0)->Coordinates);
    m_ClosureContour->Update();
  }

  if (m_ClosureContour->GetNumberOfVertices() == 2)
  {
    m_ClosureContour->RemoveVertexAt(0);
  }

  m_ClosureContour->AddVertexAtFront(endpoint);
}

void mitk::EditableContourTool::EnableContourInteraction(bool on)
{
  if (m_ContourInteractor.IsNotNull())
  {
    m_ContourInteractor->EnableInteraction(on);
  }
}

void mitk::EditableContourTool::ReleaseInteractors()
{
  this->EnableContourInteraction(false);
  m_ContourInteractor = nullptr;
}

mitk::ContourModel* mitk::EditableContourTool::GetContour()
{
  if (this->m_ContourNode.IsNotNull())
  {
    return dynamic_cast<ContourModel*>(this->m_ContourNode->GetData());
  }
  return nullptr;
};

const mitk::ContourModel* mitk::EditableContourTool::GetContour() const
{
  if (this->m_ContourNode.IsNotNull())
  {
    return dynamic_cast<const ContourModel*>(this->m_ContourNode->GetData());
  }
  return nullptr;
};