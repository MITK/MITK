/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkContourModelUtils.h>
#include <mitkLiveWireTool2D.h>
#include <mitkLiveWireTool2D.xpm>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

#include <type_traits>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, LiveWireTool2D, "LiveWire tool");
}

mitk::LiveWireTool2D::LiveWireTool2D()
  : EditableContourTool(), m_SnapClosureContour(false), m_CreateAndUseDynamicCosts(false)
{
}

mitk::LiveWireTool2D::~LiveWireTool2D()
{
  this->ClearSegmentation();
}

void mitk::LiveWireTool2D::ConnectActionsAndFunctions()
{
  mitk::EditableContourTool::ConnectActionsAndFunctions();
  CONNECT_FUNCTION("MovePoint", OnMouseMoveNoDynamicCosts);
}

void mitk::LiveWireTool2D::ReleaseInteractors()
{
  this->EnableContourInteraction(false);
  m_LiveWireInteractors.clear();
}

const char **mitk::LiveWireTool2D::GetXPM() const
{
  return mitkLiveWireTool2D_xpm;
}

us::ModuleResource mitk::LiveWireTool2D::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("LiveWire_48x48.png");
}

us::ModuleResource mitk::LiveWireTool2D::GetCursorIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("LiveWire_Cursor_32x32.png");
}

const char *mitk::LiveWireTool2D::GetName() const
{
  return "Live Wire";
}

void mitk::LiveWireTool2D::UpdateLiveWireContour()
{
  if (m_Contour.IsNotNull())
  {
    auto timeGeometry = m_Contour->GetTimeGeometry()->Clone();
    m_PreviewContour = this->m_LiveWireFilter->GetOutput();
    m_PreviewContour->SetTimeGeometry(timeGeometry); // needed because the results of the filter are always from 0 ms
                                                      // to 1 ms and the filter also resets its outputs.
    m_PreviewContourNode->SetData(this->m_PreviewContour);

    if (m_SnapClosureContour)
    {
      m_ClosureContour = this->m_LiveWireFilterClosure->GetOutput();
    }

    m_ClosureContour->SetTimeGeometry(timeGeometry); // needed because the results of the filter are always from 0 ms
                                                     // to 1 ms and the filter also resets its outputs.
    m_ClosureContourNode->SetData(this->m_ClosureContour);
  }
}

void mitk::LiveWireTool2D::OnTimePointChanged()
{
  auto reference = this->GetReferenceData();
  if (nullptr == reference || m_PlaneGeometry.IsNull() || m_LiveWireFilter.IsNull() || m_PreviewContourNode.IsNull())
    return;

  auto timeStep = reference->GetTimeGeometry()->TimePointToTimeStep(this->GetLastTimePointTriggered());

  m_ReferenceDataSlice = GetAffectedImageSliceAs2DImageByTimePoint(m_PlaneGeometry, reference, timeStep);
  m_LiveWireFilter->SetInput(m_ReferenceDataSlice);

  m_LiveWireFilter->Update();

  m_LiveWireFilterClosure->SetInput(m_ReferenceDataSlice);

  m_LiveWireFilterClosure->Update();

  this->UpdateLiveWireContour();

  RenderingManager::GetInstance()->RequestUpdateAll();
};


void mitk::LiveWireTool2D::EnableContourInteraction(bool on)
{
  for (const auto &interactor : m_LiveWireInteractors)
    interactor->EnableInteraction(on);
}


void mitk::LiveWireTool2D::SetSnapClosureContour(bool snap)
{
  m_SnapClosureContour = snap;
  if (!m_PreviewContour || !m_Contour)
  {
    return;
  }

  if (m_PreviewContour->GetNumberOfVertices() > 0)
  {
    UpdateClosureContour(m_PreviewContour->GetVertexAt(m_PreviewContour->GetNumberOfVertices() - 1)->Coordinates);
  }
  else if (m_Contour->GetNumberOfVertices() > 0)
  {
    UpdateClosureContour(m_Contour->GetVertexAt(m_Contour->GetNumberOfVertices() - 1)->Coordinates);
  }

  this->UpdateLiveWireContour();
}

void mitk::LiveWireTool2D::OnInitContour(StateMachineAction *s, InteractionEvent *interactionEvent)
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

  // Set current slice as input for ImageToLiveWireContourFilter
  m_LiveWireFilter = ImageLiveWireContourModelFilter::New();
  m_LiveWireFilter->SetUseCostFunction(true);
  m_LiveWireFilter->SetInput(m_ReferenceDataSlice);

  m_LiveWireFilterClosure = ImageLiveWireContourModelFilter::New();
  m_LiveWireFilterClosure->SetUseCostFunction(true);
  m_LiveWireFilterClosure->SetInput(m_ReferenceDataSlice);

  // Map click to pixel coordinates
  auto click = positionEvent->GetPositionInWorld();
  itk::Index<3> idx;
  m_ReferenceDataSlice->GetGeometry()->WorldToIndex(click, idx);

  // Get the pixel with the highest gradient in a 7x7 region
  itk::Index<3> indexWithHighestGradient;
  AccessFixedDimensionByItk_2(m_ReferenceDataSlice, FindHighestGradientMagnitudeByITK, 2, idx, indexWithHighestGradient);

  click[0] = indexWithHighestGradient[0];
  click[1] = indexWithHighestGradient[1];
  click[2] = indexWithHighestGradient[2];
  m_ReferenceDataSlice->GetGeometry()->IndexToWorld(click, click);

  // Set initial start point
  m_Contour->AddVertex(click, true);
  m_LiveWireFilter->SetStartPoint(click);
  m_LiveWireFilterClosure->SetEndPoint(click);

  if (!m_SnapClosureContour)
  {
    m_ClosureContour->AddVertex(click);
  }

  m_CreateAndUseDynamicCosts = true;

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::LiveWireTool2D::OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // Complete LiveWire interaction for the last segment. Add current LiveWire contour to
  // the finished contour and reset to start a new segment and computation.

  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (nullptr == positionEvent)
    return;

  if (m_PlaneGeometry.IsNotNull())
  {
    // Check if the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::sqrteps)
      return;
  }

  // Add repulsive points to avoid getting the same path again
  std::for_each(m_PreviewContour->IteratorBegin(), m_PreviewContour->IteratorEnd(), [this](ContourElement::VertexType *vertex) {
    ImageLiveWireContourModelFilter::InternalImageType::IndexType idx;
    this->m_ReferenceDataSlice->GetGeometry()->WorldToIndex(vertex->Coordinates, idx);
    this->m_LiveWireFilter->AddRepulsivePoint(idx);
    this->m_LiveWireFilterClosure->AddRepulsivePoint(idx);
  });

  // Remove duplicate first vertex, it's already contained in m_Contour
  m_PreviewContour->RemoveVertexAt(0);

  // Set last point as control point
  m_PreviewContour->SetControlVertexAt(m_PreviewContour->GetNumberOfVertices() - 1);

  // Merge contours
  m_Contour->Concatenate(m_PreviewContour);

  // Clear the LiveWire contour and reset the corresponding DataNode
  m_PreviewContour->Clear();

  // Set new start point
  m_LiveWireFilter->SetStartPoint(positionEvent->GetPositionInWorld());
  m_LiveWireFilterClosure->SetStartPoint(positionEvent->GetPositionInWorld());

  if (m_CreateAndUseDynamicCosts)
  {
    // Use dynamic cost map for next update
    m_LiveWireFilter->CreateDynamicCostMap(m_Contour);
    m_LiveWireFilter->SetUseDynamicCostMap(true);

    m_LiveWireFilterClosure->CreateDynamicCostMap(m_Contour);
    m_LiveWireFilterClosure->SetUseDynamicCostMap(true);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::LiveWireTool2D::OnDrawing(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  m_PreviewContour->AddVertex(positionEvent->GetPositionInWorld());
  m_LiveWireFilter->Update();

  UpdateClosureContour(positionEvent->GetPositionInWorld());

  m_CurrentRestrictedArea->AddVertex(positionEvent->GetPositionInWorld());

  this->UpdateLiveWireContour();

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::LiveWireTool2D::OnEndDrawing(StateMachineAction *s, InteractionEvent *interactionEvent)
{
  if (m_CurrentRestrictedArea->GetNumberOfVertices() > 1)
  {
    auto restrictedArea = m_CurrentRestrictedArea->Clone();
    m_RestrictedAreas.push_back(restrictedArea);
    OnAddPoint(s, interactionEvent);
  }
  m_CurrentRestrictedArea = this->CreateNewContour();
}

void mitk::LiveWireTool2D::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // Compute LiveWire segment from last control point to current mouse position

  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (nullptr == positionEvent)
    return;

  if (m_PlaneGeometry.IsNotNull())
  {
    // Check if the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::sqrteps)
      return;
  }

  m_LiveWireFilter->SetEndPoint(positionEvent->GetPositionInWorld());
  m_LiveWireFilter->Update();

  UpdateClosureContour(positionEvent->GetPositionInWorld());

  this->UpdateLiveWireContour();

  RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::LiveWireTool2D::UpdateClosureContour(mitk::Point3D endpoint)
{
  if (m_SnapClosureContour)
  {
    m_LiveWireFilterClosure->SetStartPoint(endpoint);
    m_LiveWireFilterClosure->Update();
  }
  else
  {
    mitk::EditableContourTool::UpdateClosureContour(endpoint);
  }
}

void mitk::LiveWireTool2D::OnMouseMoveNoDynamicCosts(StateMachineAction *, InteractionEvent *interactionEvent)
{
  m_LiveWireFilter->SetUseDynamicCostMap(false);
  m_LiveWireFilterClosure->SetUseDynamicCostMap(false);
  this->OnMouseMoved(nullptr, interactionEvent);
  m_LiveWireFilter->SetUseDynamicCostMap(true);
  m_LiveWireFilterClosure->SetUseDynamicCostMap(true);
}

bool mitk::LiveWireTool2D::OnCheckPoint(const InteractionEvent *interactionEvent)
{
  // Check double click on first control point to finish the LiveWire tool

  auto positionEvent = dynamic_cast<const mitk::InteractionPositionEvent *>(interactionEvent);

  if (nullptr == positionEvent)
    return false;

  mitk::Point3D click = positionEvent->GetPositionInWorld();
  mitk::Point3D first = this->m_Contour->GetVertexAt(0)->Coordinates;

  return first.EuclideanDistanceTo(click) < 4.5;
}

void mitk::LiveWireTool2D::OnFinish(StateMachineAction *s, InteractionEvent *interactionEvent)
{
  // Finish LiveWire tool interaction
  if (!OnCheckPoint(interactionEvent))
  {
    m_Contour->Concatenate(m_ClosureContour);
  }

  mitk::EditableContourTool::OnFinish(s, interactionEvent);

  m_LiveWireFilter->SetUseDynamicCostMap(false);
  m_LiveWireFilterClosure->SetUseDynamicCostMap(false);

  this->FinishTool();
}

void mitk::LiveWireTool2D::FinishTool()
{
  mitk::EditableContourTool::FinishTool();

  m_ContourInteractor = mitk::ContourModelLiveWireInteractor::New();
  m_ContourInteractor->SetDataNode(m_ContourNode);
  m_ContourInteractor->LoadStateMachine("ContourModelModificationInteractor.xml", us::GetModuleContext()->GetModule());
  m_ContourInteractor->SetEventConfig("ContourModelModificationConfig.xml", us::GetModuleContext()->GetModule());
  m_ContourInteractor->SetWorkingImage(this->m_ReferenceDataSlice);
  m_ContourInteractor->SetEditingContourModelNode(this->m_EditingContourNode);
  m_ContourInteractor->SetRestrictedAreas(this->m_RestrictedAreas);

  m_ContourNode->SetDataInteractor(m_ContourInteractor.GetPointer());

  this->m_LiveWireInteractors.push_back(m_ContourInteractor);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::LiveWireTool2D::FindHighestGradientMagnitudeByITK(itk::Image<TPixel, VImageDimension> *inputImage,
                                                             itk::Index<3> &index,
                                                             itk::Index<3> &returnIndex)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;

  const auto MAX_X = inputImage->GetLargestPossibleRegion().GetSize()[0];
  const auto MAX_Y = inputImage->GetLargestPossibleRegion().GetSize()[1];

  returnIndex[0] = index[0];
  returnIndex[1] = index[1];
  returnIndex[2] = 0.0;

  double gradientMagnitude = 0.0;
  double maxGradientMagnitude = 0.0;

  // The size and thus the region of 7x7 is only used to calculate the gradient magnitude in that region,
  // not for searching the maximum value.

  // Maximum value in each direction for size
  typename InputImageType::SizeType size;
  size[0] = 7;
  size[1] = 7;

  // Minimum value in each direction for startRegion
  IndexType startRegion;
  startRegion[0] = index[0] - 3;
  startRegion[1] = index[1] - 3;
  if (startRegion[0] < 0)
    startRegion[0] = 0;
  if (startRegion[1] < 0)
    startRegion[1] = 0;
  if (MAX_X - index[0] < 7)
    startRegion[0] = MAX_X - 7;
  if (MAX_Y - index[1] < 7)
    startRegion[1] = MAX_Y - 7;

  index[0] = startRegion[0] + 3;
  index[1] = startRegion[1] + 3;

  typename InputImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(startRegion);

  typedef typename itk::GradientMagnitudeImageFilter<InputImageType, InputImageType> GradientMagnitudeFilterType;
  typename GradientMagnitudeFilterType::Pointer gradientFilter = GradientMagnitudeFilterType::New();
  gradientFilter->SetInput(inputImage);
  gradientFilter->GetOutput()->SetRequestedRegion(region);

  gradientFilter->Update();
  typename InputImageType::Pointer gradientMagnitudeImage;
  gradientMagnitudeImage = gradientFilter->GetOutput();

  IndexType currentIndex;
  currentIndex[0] = 0;
  currentIndex[1] = 0;

  // Search max (approximate) gradient magnitude
  for (int x = -1; x <= 1; ++x)
  {
    currentIndex[0] = index[0] + x;

    for (int y = -1; y <= 1; ++y)
    {
      currentIndex[1] = index[1] + y;
      gradientMagnitude = gradientMagnitudeImage->GetPixel(currentIndex);

      // Check for new max
      if (maxGradientMagnitude < gradientMagnitude)
      {
        maxGradientMagnitude = gradientMagnitude;
        returnIndex[0] = currentIndex[0];
        returnIndex[1] = currentIndex[1];
        returnIndex[2] = 0.0;
      }
    }

    currentIndex[1] = index[1];
  }
}
