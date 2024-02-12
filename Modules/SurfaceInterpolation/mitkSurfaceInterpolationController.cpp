/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSurfaceInterpolationController.h>

#include <mitkCreateDistanceImageFromSurfaceFilter.h>
#include <mitkComputeContourSetNormalsFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkLabelSetImage.h>
#include <mitkMemoryUtilities.h>
#include <mitkNodePredicateDataUID.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarCircle.h>
#include <mitkPlaneGeometry.h>
#include <mitkReduceContourSetFilter.h>

#include <vtkFieldData.h>
#include <vtkMath.h>
#include <vtkPolygon.h>

// Check whether the given contours are coplanar
bool ContoursCoplanar(mitk::SurfaceInterpolationController::ContourPositionInformation leftHandSide,
                      mitk::SurfaceInterpolationController::ContourPositionInformation rightHandSide)
{
  // Here we check two things:
  // 1. Whether the normals of both contours are at least parallel
  // 2. Whether both contours lie in the same plane

  // Check for coplanarity:
  // a. Span a vector between two points one from each contour
  // b. Calculate dot product for the vector and one of the normals
  // c. If the dot is zero the two vectors are orthogonal and the contours are coplanar

  double vec[3];
  vec[0] = leftHandSide.ContourPoint[0] - rightHandSide.ContourPoint[0];
  vec[1] = leftHandSide.ContourPoint[1] - rightHandSide.ContourPoint[1];
  vec[2] = leftHandSide.ContourPoint[2] - rightHandSide.ContourPoint[2];
  double n[3];
  n[0] = rightHandSide.ContourNormal[0];
  n[1] = rightHandSide.ContourNormal[1];
  n[2] = rightHandSide.ContourNormal[2];
  double dot = vtkMath::Dot(n, vec);

  double n2[3];
  n2[0] = leftHandSide.ContourNormal[0];
  n2[1] = leftHandSide.ContourNormal[1];
  n2[2] = leftHandSide.ContourNormal[2];

  // The normals of both contours have to be parallel but not of the same orientation
  double lengthLHS = leftHandSide.ContourNormal.GetNorm();
  double lengthRHS = rightHandSide.ContourNormal.GetNorm();
  double dot2 = vtkMath::Dot(n, n2);
  bool contoursParallel = mitk::Equal(fabs(lengthLHS * lengthRHS), fabs(dot2), 0.001);

  if (mitk::Equal(dot, 0.0, 0.001) && contoursParallel)
    return true;
  else
    return false;
}

mitk::SurfaceInterpolationController::ContourPositionInformation CreateContourPositionInformation(
  const mitk::Surface* contour, const mitk::PlaneGeometry* planeGeometry)
{
  mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo;
  contourInfo.Contour = contour;
  mitk::ScalarType n[3];
  vtkPolygon::ComputeNormal(contour->GetVtkPolyData()->GetPoints(), n);
  contourInfo.ContourNormal = n;
  contourInfo.Pos = -1;
  contourInfo.TimeStep = std::numeric_limits<long unsigned int>::max();
  contourInfo.Plane = planeGeometry;

  auto contourIntArray = vtkIntArray::SafeDownCast( contour->GetVtkPolyData()->GetFieldData()->GetAbstractArray(0) );

  if (contourIntArray->GetSize() < 2)
  {
    MITK_ERROR << "In CreateContourPositionInformation. The contourIntArray is empty.";
  }
  contourInfo.LabelValue = contourIntArray->GetValue(0);

  if (contourIntArray->GetSize() >= 3)
  {
    contourInfo.TimeStep = contourIntArray->GetValue(2);
  }

  return contourInfo;
};

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
  : m_SelectedSegmentation(nullptr),
    m_CurrentTimePoint(0.)
{
  m_DistanceImageSpacing = 0.0;
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  m_ReduceFilter->SetUseProgressBar(false);
  //  m_ReduceFilter->SetProgressStepSize(1);
  m_NormalsFilter->SetUseProgressBar(true);
  m_NormalsFilter->SetProgressStepSize(1);
  m_InterpolateSurfaceFilter->SetUseProgressBar(true);
  m_InterpolateSurfaceFilter->SetProgressStepSize(7);

  m_Contours = Surface::New();

  m_InterpolationResult = nullptr;
  m_CurrentNumberOfReducedContours = 0;
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  this->RemoveObservers();
}

void mitk::SurfaceInterpolationController::RemoveObservers()
{
  // Removing all observers
  auto dataIter = m_SegmentationObserverTags.begin();
  for (; dataIter != m_SegmentationObserverTags.end(); ++dataIter)
  {
    (*dataIter).first->RemoveObserver((*dataIter).second);
  }
  m_SegmentationObserverTags.clear();

  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNotNull())
  {
    selectedSegmentation->RemoveLabelRemovedListener(mitk::MessageDelegate1<SurfaceInterpolationController, mitk::LabelSetImage::LabelValueType>(
      this, &SurfaceInterpolationController::OnRemoveLabel));
  }
}

mitk::SurfaceInterpolationController *mitk::SurfaceInterpolationController::GetInstance()
{
  static mitk::SurfaceInterpolationController::Pointer m_Instance;

  if (m_Instance.IsNull())
  {
    m_Instance = SurfaceInterpolationController::New();
  }
  return m_Instance;
}

void mitk::SurfaceInterpolationController::AddNewContour(mitk::Surface::Pointer newContour)
{
  if (newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    ContourPositionInformation contourInfo = CreateContourPositionInformation(newContour, nullptr);
    this->AddToInterpolationPipeline(contourInfo);
    this->Modified();
  }
}

void mitk::SurfaceInterpolationController::AddNewContours(const std::vector<mitk::Surface::Pointer>& newContours,
                                                          std::vector<const mitk::PlaneGeometry*>& contourPlanes,
                                                          bool reinitializationAction)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull()) return;

  if (newContours.size() != contourPlanes.size())
  {
    MITK_ERROR << "SurfaceInterpolationController::AddNewContours. contourPlanes and newContours are not of the same size.";
  }

  for (size_t i = 0; i < newContours.size(); ++i)
  {
    const auto &newContour = newContours[i];

    const mitk::PlaneGeometry * planeGeometry = contourPlanes[i];
    if (newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
    {
      auto contourInfo = CreateContourPositionInformation(newContour, planeGeometry);
      if (!reinitializationAction)
      {
        contourInfo.ContourPoint = this->ComputeInteriorPointOfContour(contourInfo, selectedSegmentation);
      }
      else
      {
        auto vtkPolyData = contourInfo.Contour->GetVtkPolyData();
        auto pointVtkArray = vtkDoubleArray::SafeDownCast(vtkPolyData->GetFieldData()->GetAbstractArray(1));
        mitk::ScalarType *ptArr = new mitk::ScalarType[3];
        for (int i = 0; i < pointVtkArray->GetSize(); ++i)
          ptArr[i] = pointVtkArray->GetValue(i);

        mitk::Point3D pt3D;
        pt3D.FillPoint(ptArr);
        contourInfo.ContourPoint = pt3D;
      }

      this->AddToInterpolationPipeline(contourInfo, reinitializationAction);
    }
  }
  this->Modified();
}

mitk::DataNode* mitk::SurfaceInterpolationController::GetSegmentationImageNode()
{
  if (m_DataStorage.IsNull()) return nullptr;
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull()) return nullptr;

  DataNode* segmentationNode = nullptr;
  mitk::NodePredicateDataUID::Pointer dataUIDPredicate = mitk::NodePredicateDataUID::New(selectedSegmentation->GetUID());
  auto dataNodeObjects = m_DataStorage->GetSubset(dataUIDPredicate);

  if (dataNodeObjects->Size() != 0)
  {
    for (auto it = dataNodeObjects->Begin(); it != dataNodeObjects->End(); ++it)
    {
      segmentationNode = it->Value();
    }
  }
  else
  {
    MITK_ERROR << "Unable to find the labelSetImage with the desired UID.";
  }
  return segmentationNode;
}

void mitk::SurfaceInterpolationController::AddPlaneGeometryNodeToDataStorage(const ContourPositionInformation& contourInfo)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow()<< "Cannot add plane geometries. No valid segmentation selected.";
  }

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested in AddPlaneGeometryNodeToDataStorage.";
    return;
  }

  auto planeGeometry = contourInfo.Plane;
  if (planeGeometry)
  {
    auto planeGeometryData = mitk::PlanarCircle::New();
    planeGeometryData->SetPlaneGeometry(planeGeometry->Clone());
    mitk::Point2D p1;
    planeGeometry->Map(planeGeometry->GetCenter(), p1);
    planeGeometryData->PlaceFigure(p1);
    planeGeometryData->SetCurrentControlPoint(p1);
    planeGeometryData->SetProperty("initiallyplaced", mitk::BoolProperty::New(true));

    auto segmentationNode = this->GetSegmentationImageNode();
    auto isContourPlaneGeometry = mitk::NodePredicateProperty::New("isContourPlaneGeometry", mitk::BoolProperty::New(true));

    mitk::DataStorage::SetOfObjects::ConstPointer contourNodes =
      m_DataStorage->GetDerivations(segmentationNode, isContourPlaneGeometry);

    mitk::DataNode::Pointer contourPlaneGeometryDataNode;

    //  Go through the pre-existing contours and check if the contour position matches them.
    for (auto it = contourNodes->Begin(); it != contourNodes->End(); ++it)
    {
      auto labelID = dynamic_cast<mitk::UShortProperty *>(it->Value()->GetProperty("labelID"))->GetValue();
      auto posID = dynamic_cast<mitk::IntProperty *>(it->Value()->GetProperty("position"))->GetValue();
      bool sameLabel = (labelID == contourInfo.LabelValue);
      bool samePos = (posID == contourInfo.Pos);

      if (samePos && sameLabel)
      {
        contourPlaneGeometryDataNode = it->Value();
        break;
      }
    }

    //  Go through the contourPlaneGeometry Data and add the segmentationNode to it.
    if (contourPlaneGeometryDataNode.IsNull())
    {
      const auto currentTimeStep = selectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

      std::string contourName = "contourPlane T " + std::to_string(currentTimeStep) + " L " + std::to_string(contourInfo.LabelValue) + "P " + std::to_string(contourInfo.Pos);

      contourPlaneGeometryDataNode = mitk::DataNode::New();
      contourPlaneGeometryDataNode->SetData(planeGeometryData);

      //  No need to change properties
      contourPlaneGeometryDataNode->SetProperty("helper object", mitk::BoolProperty::New(false));
      contourPlaneGeometryDataNode->SetProperty("hidden object", mitk::BoolProperty::New(true));
      contourPlaneGeometryDataNode->SetProperty("isContourPlaneGeometry", mitk::BoolProperty::New(true));
      contourPlaneGeometryDataNode->SetVisibility(false);

      //  Need to change properties
      contourPlaneGeometryDataNode->SetProperty("name", mitk::StringProperty::New(contourName) );
      contourPlaneGeometryDataNode->SetProperty("labelID", mitk::UShortProperty::New(contourInfo.LabelValue));
      contourPlaneGeometryDataNode->SetProperty("position", mitk::IntProperty::New(contourInfo.Pos));
      contourPlaneGeometryDataNode->SetProperty("timeStep", mitk::IntProperty::New(currentTimeStep));

      contourPlaneGeometryDataNode->SetProperty("px", mitk::DoubleProperty::New(contourInfo.ContourPoint[0]));
      contourPlaneGeometryDataNode->SetProperty("py", mitk::DoubleProperty::New(contourInfo.ContourPoint[1]));
      contourPlaneGeometryDataNode->SetProperty("pz", mitk::DoubleProperty::New(contourInfo.ContourPoint[2]));

      m_DataStorage->Add(contourPlaneGeometryDataNode, segmentationNode);
    }

    contourPlaneGeometryDataNode->SetData(planeGeometryData);

  }
}

void mitk::SurfaceInterpolationController::AddToInterpolationPipeline(ContourPositionInformation& contourInfo, bool reinitializationAction)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
    return;

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
    return;
  }

  if (contourInfo.Plane == nullptr)
  {
    MITK_ERROR << "contourInfo plane is null.";
    return;
  }

  if (contourInfo.Contour->GetVtkPolyData()->GetNumberOfPoints() == 0)
  {
    this->RemoveContour(contourInfo);
    MITK_DEBUG << "contourInfo contour is empty.";
    return;
  }

  {
    std::lock_guard<std::shared_mutex> guard(m_CPIMutex);
    const auto currentTimeStep = reinitializationAction ? contourInfo.TimeStep : selectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

    const auto currentLabelValue = reinitializationAction ? contourInfo.LabelValue : selectedSegmentation->GetActiveLabel()->GetValue();

    auto& currentImageContours = m_CPIMap[selectedSegmentation];
    auto& currentLabelContours = currentImageContours[currentLabelValue];
    auto& currentContourList = currentLabelContours[currentTimeStep];

    auto finding = std::find_if(currentContourList.begin(), currentContourList.end(), [contourInfo](const ContourPositionInformation& element) {return ContoursCoplanar(contourInfo, element); });

    if (finding != currentContourList.end())
    {
      contourInfo.Pos = finding->Pos;
      *finding = contourInfo;
    }
    else
    {
      contourInfo.Pos = currentContourList.size();
      currentContourList.push_back(contourInfo);
    }
  }

  if (!reinitializationAction)
  {
    this->AddPlaneGeometryNodeToDataStorage(contourInfo);
  }
}

bool mitk::SurfaceInterpolationController::RemoveContour(ContourPositionInformation contourInfo)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    return false;
  }

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    return false;
  }

  bool removedIt = false;

  {
    std::lock_guard<std::shared_mutex> cpiGuard(m_CPIMutex);

    const auto currentTimeStep = selectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);
    const auto currentLabel = selectedSegmentation->GetActiveLabel()->GetValue();
    auto it = m_CPIMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep).begin();


    while (it != m_CPIMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep).end())
    {
      const ContourPositionInformation& currentContour = (*it);
      if (ContoursCoplanar(currentContour, contourInfo))
      {
        m_CPIMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep).erase(it);
        removedIt = true;
        break;
      }
      ++it;
    }
  }

  if (removedIt)
  {
    this->ReinitializeInterpolation();
  }

  return removedIt;
}

void mitk::SurfaceInterpolationController::AddActiveLabelContoursForInterpolation(mitk::Label::PixelType activeLabel)
{
  this->ReinitializeInterpolation();

  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow() << "Cannot add active label contours. No valid segmentation selected.";
  }

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
    return;
  }

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
    return;
  }
  const auto currentTimeStep = selectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

  std::shared_lock<std::shared_mutex> guard(m_CPIMutex);

  auto& currentImageContours = m_CPIMap.at(selectedSegmentation);

  auto finding = currentImageContours.find(activeLabel);
  if (finding == currentImageContours.end())
  {
    MITK_INFO << "Contours for label don't exist. Label value: " << activeLabel;
    return;
  }

  auto currentLabelContoursMap = finding->second;

  auto tsfinding = currentLabelContoursMap.find(currentTimeStep);
  if (tsfinding == currentLabelContoursMap.end())
  {
    MITK_INFO << "Contours for current time step don't exist.";
    return;
  }

  CPIVector& currentContours = tsfinding->second;

  unsigned int index = 0;
  m_ReduceFilter->Reset();
  for (const auto&  cpi : currentContours)
  {
    m_ReduceFilter->SetInput(index, cpi.Contour);
    ++index;
  }
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow() << "Cannot interpolate. No valid segmentation selected.";
  }

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_WARN << "No interpolation possible, currently selected timepoint is not in the time bounds of currently selected segmentation. Time point: " << m_CurrentTimePoint;
    m_InterpolationResult = nullptr;
    return;
  }
  const auto currentTimeStep = selectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);
  m_ReduceFilter->Update();
  m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

  if (m_CurrentNumberOfReducedContours == 1)
  {
    vtkPolyData *tmp = m_ReduceFilter->GetOutput(0)->GetVtkPolyData();
    if (tmp == nullptr)
    {
      m_CurrentNumberOfReducedContours = 0;
    }
  }

  //  We use the timeSelector to get the segmentation image for the current segmentation.
  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(selectedSegmentation);
  timeSelector->SetTimeNr(currentTimeStep);
  timeSelector->SetChannelNr(0);
  timeSelector->Update();

  mitk::Image::Pointer refSegImage = timeSelector->GetOutput();
  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
  AccessFixedDimensionByItk_1(refSegImage, GetImageBase, 3, itkImage);

  m_NormalsFilter->SetSegmentationBinaryImage(refSegImage);

  for (size_t i = 0; i < m_CurrentNumberOfReducedContours; ++i)
  {
    mitk::Surface::Pointer reducedContour = m_ReduceFilter->GetOutput(i);
    reducedContour->DisconnectPipeline();
    m_NormalsFilter->SetInput(i, reducedContour);
    m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
  }

  if (m_CurrentNumberOfReducedContours < 2)
  {
    // If no interpolation is possible reset the interpolation result
    MITK_INFO << "Interpolation impossible: not enough contours.";
    m_InterpolationResult = nullptr;
    return;
  }

  // Setting up progress bar
  mitk::ProgressBar::GetInstance()->AddStepsToDo(10);

  // create a surface from the distance-image
  mitk::ImageToSurfaceFilter::Pointer imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
  imageToSurfaceFilter->SetInput(m_InterpolateSurfaceFilter->GetOutput());
  imageToSurfaceFilter->SetThreshold(0);
  imageToSurfaceFilter->SetSmooth(true);
  imageToSurfaceFilter->SetSmoothIteration(1);
  imageToSurfaceFilter->Update();

  mitk::Surface::Pointer interpolationResult = mitk::Surface::New();
  interpolationResult->Expand(selectedSegmentation->GetTimeSteps());

  auto geometry = selectedSegmentation->GetTimeGeometry()->Clone();
  geometry->ReplaceTimeStepGeometries(mitk::Geometry3D::New());
  interpolationResult->SetTimeGeometry(geometry);

  interpolationResult->SetVtkPolyData(imageToSurfaceFilter->GetOutput()->GetVtkPolyData(), currentTimeStep);
  m_InterpolationResult = interpolationResult;

  m_DistanceImageSpacing = m_InterpolateSurfaceFilter->GetDistanceImageSpacing();

  auto* contoursGeometry = static_cast<mitk::ProportionalTimeGeometry*>(m_Contours->GetTimeGeometry());
  auto timeBounds = geometry->GetTimeBounds(currentTimeStep);
  contoursGeometry->SetFirstTimePoint(timeBounds[0]);
  contoursGeometry->SetStepDuration(timeBounds[1] - timeBounds[0]);

  // Last progress step
  mitk::ProgressBar::GetInstance()->Progress(20);
  m_InterpolationResult->DisconnectPipeline();
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::GetInterpolationResult()
{
  return m_InterpolationResult;
}

mitk::Surface *mitk::SurfaceInterpolationController::GetContoursAsSurface()
{
  return m_Contours;
}


void mitk::SurfaceInterpolationController::SetDataStorage(DataStorage::Pointer ds)
{
  m_DataStorage = ds;
}

void mitk::SurfaceInterpolationController::SetMinSpacing(double minSpacing)
{
  m_ReduceFilter->SetMinSpacing(minSpacing);
}

void mitk::SurfaceInterpolationController::SetMaxSpacing(double maxSpacing)
{
  m_ReduceFilter->SetMaxSpacing(maxSpacing);
  m_NormalsFilter->SetMaxSpacing(maxSpacing);
}

void mitk::SurfaceInterpolationController::SetDistanceImageVolume(unsigned int distImgVolume)
{
  m_InterpolateSurfaceFilter->SetDistanceImageVolume(distImgVolume);
}

mitk::LabelSetImage* mitk::SurfaceInterpolationController::GetCurrentSegmentation()
{
  return m_SelectedSegmentation.Lock();
}

mitk::Image *mitk::SurfaceInterpolationController::GetInterpolationImage()
{
  return m_InterpolateSurfaceFilter->GetOutput();
}

double mitk::SurfaceInterpolationController::EstimatePortionOfNeededMemory()
{
  double numberOfPointsAfterReduction = m_ReduceFilter->GetNumberOfPointsAfterReduction() * 3;
  double sizeOfPoints = pow(numberOfPointsAfterReduction, 2) * sizeof(double);
  double totalMem = mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  double percentage = sizeOfPoints / totalMem;
  return percentage;
}

unsigned int mitk::SurfaceInterpolationController::GetNumberOfInterpolationSessions()
{
  return m_CPIMap.size();
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::SurfaceInterpolationController::GetImageBase(itk::Image<TPixel, VImageDimension> *input,
                                                        itk::ImageBase<3>::Pointer &result)
{
  result->Graft(input);
}

void mitk::SurfaceInterpolationController::SetCurrentInterpolationSession(mitk::LabelSetImage* currentSegmentationImage)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (currentSegmentationImage == selectedSegmentation)
  {
    return;
  }

  if (selectedSegmentation.IsNotNull())
  {
    selectedSegmentation->RemoveLabelRemovedListener(mitk::MessageDelegate1<SurfaceInterpolationController, mitk::LabelSetImage::LabelValueType>(
      this, &SurfaceInterpolationController::OnRemoveLabel));
  }

  m_SelectedSegmentation = currentSegmentationImage;
  selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation.IsNotNull())
  {
    std::lock_guard<std::shared_mutex> guard(m_CPIMutex);

    auto it = m_CPIMap.find(selectedSegmentation);
    if (it == m_CPIMap.end())
    {
      m_CPIMap[selectedSegmentation] = CPITimeStepLabelMap();

      auto command = itk::MemberCommand<SurfaceInterpolationController>::New();
      command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
      m_SegmentationObserverTags[selectedSegmentation] = selectedSegmentation->AddObserver(itk::DeleteEvent(), command);
    }

    selectedSegmentation->AddLabelRemovedListener(mitk::MessageDelegate1<SurfaceInterpolationController, mitk::LabelSetImage::LabelValueType>(
      this, &SurfaceInterpolationController::OnRemoveLabel));
  }

  m_InterpolationResult = nullptr;
  m_CurrentNumberOfReducedContours = 0;
  m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
  this->ReinitializeInterpolation();
}

void mitk::SurfaceInterpolationController::RemoveInterpolationSession(mitk::LabelSetImage* segmentationImage)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (nullptr != segmentationImage)
  {
    if (selectedSegmentation == segmentationImage)
    {
      selectedSegmentation->RemoveLabelRemovedListener(mitk::MessageDelegate1<SurfaceInterpolationController, mitk::LabelSetImage::LabelValueType>(
        this, &SurfaceInterpolationController::OnRemoveLabel));
      m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
      m_SelectedSegmentation = nullptr;
    }

    {
      std::lock_guard<std::shared_mutex> guard(m_CPIMutex);
      m_CPIMap.erase(segmentationImage);
    }

    // Remove observer
    auto pos = m_SegmentationObserverTags.find(segmentationImage);
    if (pos != m_SegmentationObserverTags.end())
    {
      segmentationImage->RemoveObserver((*pos).second);
      m_SegmentationObserverTags.erase(pos);
    }
  }
}

void mitk::SurfaceInterpolationController::RemoveAllInterpolationSessions()
{
  this->RemoveObservers();
  m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
  m_SelectedSegmentation = nullptr;

  std::lock_guard<std::shared_mutex> guard(m_CPIMutex);
  m_CPIMap.clear();
}

void mitk::SurfaceInterpolationController::RemoveContours(mitk::Label::PixelType label,
  TimeStepType timeStep)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow() << "Cannot remove contours. No valid segmentation selected.";
  }

  std::lock_guard<std::shared_mutex> guard(m_CPIMutex);

  auto cpiLabelMap = m_CPIMap[selectedSegmentation];
  auto finding = cpiLabelMap.find(label);

  if (finding != cpiLabelMap.end())
  {
    finding->second.erase(timeStep);
  }
}

void mitk::SurfaceInterpolationController::RemoveContours(mitk::Label::PixelType label)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow() << "Cannot remove contours. No valid segmentation selected.";
  }

  std::lock_guard<std::shared_mutex> guard(m_CPIMutex);
  m_CPIMap[selectedSegmentation].erase(label);
}


void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller,
                                                                 const itk::EventObject & /*event*/)
{
  auto *tempImage = dynamic_cast<mitk::LabelSetImage *>(const_cast<itk::Object *>(caller));
  if (tempImage)
  {
    std::lock_guard<std::shared_mutex> guard(m_CPIMutex);

    auto selectedSegmentation = m_SelectedSegmentation.Lock();

    if (selectedSegmentation == tempImage)
    {
      this->SetCurrentInterpolationSession(nullptr);
    }
    m_SegmentationObserverTags.erase(tempImage);
    m_CPIMap.erase(tempImage);
  }
}

void mitk::SurfaceInterpolationController::ReinitializeInterpolation()
{
  // If session has changed reset the pipeline
  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();

  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();

  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation.IsNotNull())
  {
    if (!selectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
    {
      MITK_WARN << "Interpolation cannot be reinitialized. Currently selected timepoint is not in the time bounds of the currently selected segmentation. Time point: " << m_CurrentTimePoint;
      return;
    }

    const auto currentTimeStep = selectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

    //  Set reference image for interpolation surface filter
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(selectedSegmentation);
    timeSelector->SetTimeNr(currentTimeStep);
    timeSelector->SetChannelNr(0);
    timeSelector->Update();
    mitk::Image::Pointer refSegImage = timeSelector->GetOutput();
    AccessFixedDimensionByItk_1(refSegImage, GetImageBase, 3, itkImage);
    m_InterpolateSurfaceFilter->SetReferenceImage(itkImage.GetPointer());
  }
}

void mitk::SurfaceInterpolationController::OnRemoveLabel(mitk::Label::PixelType removedLabelValue)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation.IsNotNull())
  {
    this->RemoveContours(removedLabelValue);
  }
}

mitk::SurfaceInterpolationController::CPIVector* mitk::SurfaceInterpolationController::GetContours(TimeStepType timeStep, LabelSetImage::LabelValueType labelValue)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation == nullptr)
    return nullptr;

  std::shared_lock<std::shared_mutex> guard(m_CPIMutex);

  auto labelFinding = m_CPIMap[selectedSegmentation].find(labelValue);

  if (labelFinding != m_CPIMap[selectedSegmentation].end())
  {
    auto tsFinding = labelFinding->second.find(timeStep);

    if (tsFinding != labelFinding->second.end())
    {
      return &(tsFinding->second);
    }
  }

  return nullptr;
}

void mitk::SurfaceInterpolationController::CompleteReinitialization(const std::vector<mitk::Surface::Pointer>& contourList,
                                                                    std::vector<const mitk::PlaneGeometry *>& contourPlanes)
{
  this->ClearInterpolationSession();

  //  Now the layers should be empty and the new layers can be added.
  this->AddNewContours(contourList, contourPlanes, true);
}

void mitk::SurfaceInterpolationController::ClearInterpolationSession()
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation != nullptr)
  {
    std::lock_guard<std::shared_mutex> guard(m_CPIMutex);
    m_CPIMap[selectedSegmentation].clear();
  }
}

std::vector< mitk::Point3D > mitk::ContourExt::GetBoundingBoxGridPoints(
                                              size_t planeDimension,
                                              double startDim1,
                                              size_t numPointsToSampleDim1,
                                              double deltaDim1,
                                              double startDim2,
                                              size_t numPointsToSampleDim2,
                                              double deltaDim2,
                                              double valuePlaneDim)
{
  std::vector< mitk::Point3D > gridPoints;
  for (size_t i = 0; i < numPointsToSampleDim1; ++i)
  {
    for (size_t j = 0; j < numPointsToSampleDim2; ++j)
    {
      mitk::ScalarType *ptVec = new mitk::ScalarType[3];

      if (planeDimension == 0)
      {
        ptVec[0] = valuePlaneDim;
        ptVec[1] = startDim1 + deltaDim1 * i;
        ptVec[2] = startDim2 + deltaDim2 * j;
      }
      else if (planeDimension == 1)
      {
        ptVec[0] = startDim1 + deltaDim1 * i;
        ptVec[1] = valuePlaneDim;
        ptVec[2] = startDim2 + deltaDim2 * j;

      }
      else if (planeDimension == 2)
      {
        ptVec[0] = startDim1 + deltaDim1 * i;
        ptVec[1] = startDim2 + deltaDim2 * j;
        ptVec[2] = valuePlaneDim;
      }

      mitk::Point3D pt3D;
      pt3D.FillPoint(ptVec);
      gridPoints.push_back(pt3D);
    }
  }

  return gridPoints;
}

mitk::Point3D mitk::SurfaceInterpolationController::ComputeInteriorPointOfContour(
                                    const mitk::SurfaceInterpolationController::ContourPositionInformation& contour,
                                    mitk::LabelSetImage * labelSetImage)
{
  if (labelSetImage->GetDimension() == 4)
  {
    return mitk::ContourExt::ComputeInteriorPointOfContour<4>(contour, labelSetImage, m_CurrentTimePoint);
  }
  else
  {
    return mitk::ContourExt::ComputeInteriorPointOfContour<3>(contour, labelSetImage, m_CurrentTimePoint);
  }
}

template<unsigned int VImageDimension>
mitk::Point3D mitk::ContourExt::ComputeInteriorPointOfContour(
                                    const mitk::SurfaceInterpolationController::ContourPositionInformation& contour,
                                    mitk::LabelSetImage * labelSetImage,
                                    mitk::TimePointType currentTimePoint)
{
  mitk::ImagePixelReadAccessor<mitk::Label::PixelType, VImageDimension> readAccessor(labelSetImage);

  if (!labelSetImage->GetTimeGeometry()->IsValidTimePoint(currentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
    mitk::Point3D pt;
    return pt;
  }

  std::vector<mitk::Label::PixelType> pixelsPresent;
  const auto currentTimeStep = labelSetImage->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);

  auto polyData = contour.Contour->GetVtkPolyData();

  polyData->ComputeCellsBounds();
  mitk::ScalarType cellBounds[6];
  polyData->GetCellsBounds(cellBounds);

  size_t numPointsToSample = 10;
  mitk::ScalarType StartX = cellBounds[0];
  mitk::ScalarType StartY = cellBounds[2];
  mitk::ScalarType StartZ = cellBounds[4];

  size_t deltaX = (cellBounds[1] - cellBounds[0]) / numPointsToSample;
  size_t deltaY = (cellBounds[3] - cellBounds[2]) / numPointsToSample;
  size_t deltaZ = (cellBounds[5] - cellBounds[4]) / numPointsToSample;

  auto planeOrientation = mitk::ContourExt::GetContourOrientation(contour.ContourNormal);

  std::vector<mitk::Point3D> points;
  if (planeOrientation == 0)
  {
    points = mitk::ContourExt::GetBoundingBoxGridPoints(planeOrientation,
                                      StartY, numPointsToSample, deltaY,
                                      StartZ, numPointsToSample, deltaZ,
                                      StartX);
  }
  else if (planeOrientation == 1)
  {
    points = mitk::ContourExt::GetBoundingBoxGridPoints(planeOrientation,
                                      StartX, numPointsToSample, deltaX,
                                      StartZ, numPointsToSample, deltaZ,
                                      StartY);
  }
  else if (planeOrientation == 2)
  {
    points = mitk::ContourExt::GetBoundingBoxGridPoints(planeOrientation,
                                      StartX, numPointsToSample, deltaX,
                                      StartY, numPointsToSample, deltaY,
                                      StartZ);
  }
  mitk::Label::PixelType pixelVal;
  mitk::Point3D pt3D;
  std::vector<mitk::Label::PixelType> pixelVals;
  for (size_t i = 0; i < points.size(); ++i)
  {
    pt3D = points[i];
    itk::Index<3> itkIndex;
    labelSetImage->GetGeometry()->WorldToIndex(pt3D, itkIndex);

    if (VImageDimension == 4)
    {
      itk::Index<VImageDimension> time3DIndex;
      for (size_t i = 0; i < itkIndex.size(); ++i)
        time3DIndex[i] = itkIndex[i];
      time3DIndex[3] = currentTimeStep;

      pixelVal = readAccessor.GetPixelByIndexSafe(time3DIndex);
    }
    else if (VImageDimension == 3)
    {
      itk::Index<VImageDimension> geomIndex;
      for (size_t i=0;i<itkIndex.size();++i)
        geomIndex[i] = itkIndex[i];

      pixelVal = readAccessor.GetPixelByIndexSafe(geomIndex);
    }

    if (pixelVal == contour.LabelValue)
      break;
  }
  return pt3D;
}

size_t mitk::ContourExt::GetContourOrientation(const mitk::Vector3D& ContourNormal)
{
  double n[3];
  n[0] = ContourNormal[0];
  n[1] = ContourNormal[1];
  n[2] = ContourNormal[2];

  double XVec[3];
  XVec[0] = 1.0;  XVec[1] = 0.0;  XVec[2] = 0.0;
  double dotX = vtkMath::Dot(n, XVec);

  double YVec[3];
  YVec[0] = 0.0;  YVec[1] = 1.0;  YVec[2] = 0.0;
  double dotY = vtkMath::Dot(n, YVec);

  double ZVec[3];
  ZVec[0] = 0.0;  ZVec[1] = 0.0;  ZVec[2] = 1.0;
  double dotZ = vtkMath::Dot(n, ZVec);

  size_t planeOrientation = 0;
  if (fabs(dotZ) > mitk::eps)
  {
    planeOrientation = 2;
  }
  else if (fabs(dotY) > mitk::eps)
  {
    planeOrientation = 1;
  }
  else if(fabs(dotX) > mitk::eps)
  {
    planeOrientation = 0;
  }
  return planeOrientation;
}
