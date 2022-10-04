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
  : EditableContourTool(), m_CreateAndUseDynamicCosts(false)
{
}

mitk::LiveWireTool2D::~LiveWireTool2D()
{
}

void mitk::LiveWireTool2D::ConnectActionsAndFunctions()
{
  mitk::EditableContourTool::ConnectActionsAndFunctions();
  CONNECT_FUNCTION("MovePoint", OnMouseMoveNoDynamicCosts);
}

const char **mitk::LiveWireTool2D::GetXPM() const
{
  return mitkLiveWireTool2D_xpm;
}

us::ModuleResource mitk::LiveWireTool2D::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("LiveWire.svg");
}

us::ModuleResource mitk::LiveWireTool2D::GetCursorIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("LiveWire_Cursor.svg");
}

const char *mitk::LiveWireTool2D::GetName() const
{
  return "Live Wire";
}

void mitk::LiveWireTool2D::UpdateLiveWireContour()
{
  auto contour = this->GetContour();
  if (nullptr != contour)
  {
    auto timeGeometry = contour->GetTimeGeometry()->Clone();
    m_PreviewContour = this->m_LiveWireFilter->GetOutput();

    // needed because the results of the filter are always from 0 ms
    // to 1 ms and the filter also resets its outputs.
    // generate a time geometry that is always visible as the working contour should always be.
    auto contourTimeGeometry = ProportionalTimeGeometry::New();
    contourTimeGeometry->SetStepDuration(std::numeric_limits<TimePointType>::max());
    contourTimeGeometry->SetTimeStepGeometry(contour->GetTimeGeometry()->GetGeometryForTimeStep(0)->Clone(), 0);
    m_PreviewContour->SetTimeGeometry(contourTimeGeometry);
    m_PreviewContourNode->SetData(this->m_PreviewContour);
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

  this->UpdateLiveWireContour();

  RenderingManager::GetInstance()->RequestUpdateAll();
};


mitk::Point3D mitk::LiveWireTool2D::PrepareInitContour(const mitk::Point3D& clickedPoint)
{
  // Set current slice as input for ImageToLiveWireContourFilter
  m_LiveWireFilter = ImageLiveWireContourModelFilter::New();
  m_LiveWireFilter->SetUseCostFunction(true);
  m_LiveWireFilter->SetInput(m_ReferenceDataSlice);

  itk::Index<3> idx;
  m_ReferenceDataSlice->GetGeometry()->WorldToIndex(clickedPoint, idx);

  // Get the pixel with the highest gradient in a 7x7 region
  itk::Index<3> indexWithHighestGradient;
  AccessFixedDimensionByItk_2(m_ReferenceDataSlice, FindHighestGradientMagnitudeByITK, 2, idx, indexWithHighestGradient);

  Point3D adaptedClick;
  m_ReferenceDataSlice->GetGeometry()->IndexToWorld(indexWithHighestGradient, adaptedClick);

  m_CreateAndUseDynamicCosts = true;

  return adaptedClick;
}

void mitk::LiveWireTool2D::FinalizePreviewContour(const Point3D& clickedPoint)
{
  // Add repulsive points to avoid getting the same path again
  std::for_each(m_PreviewContour->IteratorBegin(), m_PreviewContour->IteratorEnd(), [this](ContourElement::VertexType* vertex) {
    ImageLiveWireContourModelFilter::InternalImageType::IndexType idx;
    this->m_ReferenceDataSlice->GetGeometry()->WorldToIndex(vertex->Coordinates, idx);
    this->m_LiveWireFilter->AddRepulsivePoint(idx);
  });

  EditableContourTool::FinalizePreviewContour(clickedPoint);
}

void mitk::LiveWireTool2D::InitializePreviewContour(const Point3D& clickedPoint)
{
  m_PreviewContour->Clear();

  // Set new start point
  m_LiveWireFilter->SetStartPoint(clickedPoint);

  if (m_CreateAndUseDynamicCosts)
  {
    auto contour = this->GetContour();
    // Use dynamic cost map for next update
    m_LiveWireFilter->CreateDynamicCostMap(contour);
    m_LiveWireFilter->SetUseDynamicCostMap(true);
  }
}

void mitk::LiveWireTool2D::UpdatePreviewContour(const Point3D& clickedPoint)
{   // Compute LiveWire segment from last control point to current mouse position
  m_LiveWireFilter->SetEndPoint(clickedPoint);
  m_LiveWireFilter->Update();

  this->UpdateLiveWireContour();
}

void mitk::LiveWireTool2D::OnMouseMoveNoDynamicCosts(StateMachineAction *, InteractionEvent *interactionEvent)
{
  m_LiveWireFilter->SetUseDynamicCostMap(false);
  this->OnMouseMoved(nullptr, interactionEvent);
  m_LiveWireFilter->SetUseDynamicCostMap(true);
}

void mitk::LiveWireTool2D::FinishTool()
{
  m_LiveWireFilter->SetUseDynamicCostMap(false);

  m_ContourInteractor = mitk::ContourModelLiveWireInteractor::New();
  m_ContourInteractor->SetDataNode(m_ContourNode);
  m_ContourInteractor->LoadStateMachine("ContourModelModificationInteractor.xml", us::GetModuleContext()->GetModule());
  m_ContourInteractor->SetEventConfig("ContourModelModificationConfig.xml", us::GetModuleContext()->GetModule());
  m_ContourInteractor->SetWorkingImage(this->m_ReferenceDataSlice);
  m_ContourInteractor->SetRestrictedArea(this->m_CurrentRestrictedArea);

  m_ContourNode->SetDataInteractor(m_ContourInteractor.GetPointer());
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
