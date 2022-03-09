/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEditableContourTool.h"
#include <mitkToolManager.h>


mitk::EditableContourTool::EditableContourTool() : FeedbackContourTool("EditableContourTool") {}

mitk::EditableContourTool::~EditableContourTool()
{
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
  this->ConfirmSegmentation();
  Superclass::Deactivated();
}

void mitk::EditableContourTool::ConfirmSegmentation() 
{
  auto referenceImage = this->GetReferenceData();
  auto workingImage = this->GetWorkingData();

  if (nullptr != referenceImage && nullptr != workingImage)
  {
    std::vector<SliceInformation> sliceInfos;
    sliceInfos.reserve(m_WorkingContours.size());

    const auto currentTimePoint =
      mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    TimeStepType workingImageTimeStep = workingImage->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);

    for (const auto &workingContour : m_WorkingContours)
    {
      auto contour = dynamic_cast<ContourModel *>(workingContour.first->GetData());

      if (nullptr == contour || contour->IsEmpty())
        continue;

      auto sameSlicePredicate = [&workingContour, workingImageTimeStep](const SliceInformation &si) {
        return workingContour.second->IsOnPlane(si.plane) && workingImageTimeStep == si.timestep;
      };

      auto finding = std::find_if(sliceInfos.begin(), sliceInfos.end(), sameSlicePredicate);
      if (finding == sliceInfos.end())
      {
        auto workingSlice =
          this->GetAffectedImageSliceAs2DImage(workingContour.second, workingImage, workingImageTimeStep)->Clone();
        sliceInfos.emplace_back(workingSlice, workingContour.second, workingImageTimeStep);
        finding = std::prev(sliceInfos.end());
      }

      // cast const away is OK in this case, because these are all slices created and manipulated
      // localy in this function call. And we want to keep the high constness of SliceInformation for
      // public interfaces.
      auto workingSlice = const_cast<Image *>(finding->slice.GetPointer());

      auto projectedContour = ContourModelUtils::ProjectContourTo2DSlice(workingSlice, contour);
      int activePixelValue = ContourModelUtils::GetActivePixelValue(workingImage);

      ContourModelUtils::FillContourInSlice(projectedContour, workingSlice, workingImage, activePixelValue);
    }

    this->WriteBackSegmentationResults(sliceInfos);
  }

  this->ClearSegmentation();
}

void mitk::EditableContourTool::ClearSegmentation() 
{
  this->ReleaseHelperObjects();
  this->ReleaseInteractors();
  this->ResetToStartState();
}

bool mitk::EditableContourTool::IsPositionEventInsideImageRegion(mitk::InteractionPositionEvent *positionEvent,
                                                            mitk::BaseData *data)
{
  bool isPositionEventInsideImageRegion =
    nullptr != data && data->GetGeometry()->IsInside(positionEvent->GetPositionInWorld());

  if (!isPositionEventInsideImageRegion)
    MITK_WARN("EditableContourTool") << "PositionEvent is outside ImageRegion!";

  return isPositionEventInsideImageRegion;
}

void mitk::EditableContourTool::OnInitContour(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (nullptr == positionEvent)
    return;

  auto workingDataNode = this->GetWorkingDataNode();

  if (!IsPositionEventInsideImageRegion(positionEvent, workingDataNode->GetData()))
  {
    this->ResetToStartState();
    return;
  }

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  m_Contour = this->CreateNewContour();
  m_ContourNode = mitk::DataNode::New();
  m_ContourNode->SetData(m_Contour);
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

  m_EditingContour = this->CreateNewContour();
  m_EditingContourNode = mitk::DataNode::New();
  m_EditingContourNode->SetData(m_EditingContour);
  m_EditingContourNode->SetName("editing node");
  m_EditingContourNode->SetProperty("layer", IntProperty::New(102));
  m_EditingContourNode->AddProperty("fixedLayer", BoolProperty::New(true));
  m_EditingContourNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_EditingContourNode->AddProperty("contour.color", ColorProperty::New(0.1f, 1.0f, 0.1f), nullptr, true);
  m_EditingContourNode->AddProperty("contour.points.color", ColorProperty::New(0.0f, 0.0f, 1.0f), nullptr, true);
  m_EditingContourNode->AddProperty("contour.width", mitk::FloatProperty::New(4.0f), nullptr, true);

  m_CurrentRestrictedArea = this->CreateNewContour();

  auto dataStorage = this->GetToolManager()->GetDataStorage();
  dataStorage->Add(m_ContourNode, workingDataNode);
  dataStorage->Add(m_PreviewContourNode, workingDataNode);
  dataStorage->Add(m_ClosureContourNode, workingDataNode);
  dataStorage->Add(m_EditingContourNode, workingDataNode);

  m_ReferenceDataSlice = this->GetAffectedReferenceSlice(positionEvent);

  auto origin = m_ReferenceDataSlice->GetSlicedGeometry()->GetOrigin();
  m_ReferenceDataSlice->GetSlicedGeometry()->WorldToIndex(origin, origin);
  m_ReferenceDataSlice->GetSlicedGeometry()->IndexToWorld(origin, origin);
  m_ReferenceDataSlice->GetSlicedGeometry()->SetOrigin(origin);

  // Remember PlaneGeometry to determine if events were triggered in the same plane
  m_PlaneGeometry = interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry();
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

  // remove green connection between mouse position and start point
  m_ClosureContour->Clear();

  // Save contour and corresponding plane geometry to list
  this->m_WorkingContours.emplace_back(std::make_pair(m_ContourNode, positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone()));
  this->m_EditingContours.emplace_back(std::make_pair(m_EditingContourNode, positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone()));
}

void mitk::EditableContourTool::FinishTool() 
{
  auto numberOfTimesteps = static_cast<int>(m_Contour->GetTimeSteps());

  for (int i = 0; i <= numberOfTimesteps; ++i)
    m_Contour->Close(i);

  this->GetToolManager()->GetDataStorage()->Remove(m_PreviewContourNode);

  m_PreviewContourNode = nullptr;
  m_PreviewContour = nullptr;
}

void mitk::EditableContourTool::ReleaseHelperObjects() 
{
  this->RemoveHelperObjects();

  m_EditingContours.clear();
  m_WorkingContours.clear();
  m_RestrictedAreas.clear();

  m_EditingContourNode = nullptr;
  m_EditingContour = nullptr;

  m_PreviewContourNode = nullptr;
  m_PreviewContour = nullptr;

  m_ClosureContourNode = nullptr;
  m_ClosureContour = nullptr;

  m_ContourNode = nullptr;
  m_Contour = nullptr;

  m_CurrentRestrictedArea = nullptr;
}

void mitk::EditableContourTool::RemoveHelperObjects() 
{
  auto dataStorage = this->GetToolManager()->GetDataStorage();

  if (nullptr == dataStorage)
    return;

  for (const auto &editingContour : m_EditingContours)
    dataStorage->Remove(editingContour.first);

  for (const auto &workingContour : m_WorkingContours)
    dataStorage->Remove(workingContour.first);

  if (m_EditingContourNode.IsNotNull())
    dataStorage->Remove(m_EditingContourNode);

  if (m_PreviewContourNode.IsNotNull())
    dataStorage->Remove(m_PreviewContourNode);

  if (m_ClosureContourNode.IsNotNull())
    dataStorage->Remove(m_ClosureContourNode);

  if (m_ContourNode.IsNotNull())
    dataStorage->Remove(m_ContourNode);

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
  }

  if (m_ClosureContour->GetNumberOfVertices() == 0)
  {
    m_ClosureContour->AddVertex(m_Contour->GetVertexAt(0)->Coordinates);
    m_ClosureContour->Update();
  }

  if (m_ClosureContour->GetNumberOfVertices() == 2)
  {
    m_ClosureContour->RemoveVertexAt(0);
  }

  m_ClosureContour->AddVertexAtFront(endpoint);
}
