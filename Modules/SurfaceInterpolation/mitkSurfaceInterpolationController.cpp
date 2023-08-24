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
  mitk::Surface::Pointer contour, const mitk::PlaneGeometry* planeGeometry)
{
  mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo;
  contourInfo.Contour = contour;
  mitk::ScalarType n[3];
  vtkPolygon::ComputeNormal(contour->GetVtkPolyData()->GetPoints(), n);
  contourInfo.ContourNormal = n;
  contourInfo.Pos = -1;
  contourInfo.TimeStep = std::numeric_limits<long unsigned int>::max();
  contourInfo.Plane = const_cast<mitk::PlaneGeometry *>(planeGeometry);

  auto contourIntArray = vtkIntArray::SafeDownCast( contour->GetVtkPolyData()->GetFieldData()->GetAbstractArray(0) );

  if (contourIntArray->GetSize() < 2)
  {
    MITK_ERROR << "In CreateContourPositionInformation. The contourIntArray is empty.";
  }
  contourInfo.LabelValue = contourIntArray->GetValue(0);
  contourInfo.LayerValue = contourIntArray->GetValue(1);

  if (contourIntArray->GetSize() >= 3)
  {
    contourInfo.TimeStep = contourIntArray->GetValue(2);
  }

  contourInfo.SliceIndex = 0;

  return contourInfo;
};

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
  : m_SelectedSegmentation(nullptr),
    m_CurrentTimePoint(0.),
    m_ContourIndex(0),
    m_ContourPosIndex(0),
    m_NumberOfLayersInCurrentSegmentation(0),
    m_PreviousActiveLabelValue(0),
    m_CurrentActiveLabelValue(0),
    m_PreviousLayerIndex(0),
    m_CurrentLayerIndex(0)
{
  m_DistanceImageSpacing = 0.0;
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();
  // m_TimeSelector = ImageTimeSelector::New();

  m_ReduceFilter->SetUseProgressBar(false);
  //  m_ReduceFilter->SetProgressStepSize(1);
  m_NormalsFilter->SetUseProgressBar(true);
  m_NormalsFilter->SetProgressStepSize(1);
  m_InterpolateSurfaceFilter->SetUseProgressBar(true);
  m_InterpolateSurfaceFilter->SetProgressStepSize(7);

  m_Contours = Surface::New();

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  m_PolyData->SetPoints(points);

  m_NumberOfConnectionsAdded = 0;

  m_InterpolationResult = nullptr;
  m_CurrentNumberOfReducedContours = 0;
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  // Removing all observers
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
  if (nullptr == m_SelectedSegmentation) return;

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
        contourInfo.ContourPoint = this->ComputeInteriorPointOfContour(contourInfo,
                                              dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation) );
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
    DataNode* segmentationNode = nullptr;
    mitk::NodePredicateDataUID::Pointer dataUIDPredicate = mitk::NodePredicateDataUID::New(m_SelectedSegmentation->GetUID());
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
  auto planeGeometry = contourInfo.Plane;
  auto planeGeometryData = mitk::PlanarCircle::New();
  planeGeometryData->SetPlaneGeometry(planeGeometry);
  mitk::Point2D p1;
  planeGeometry->Map(planeGeometry->GetCenter(), p1);
  planeGeometryData->PlaceFigure(p1);
  planeGeometryData->SetCurrentControlPoint(p1);
  planeGeometryData->SetProperty("initiallyplaced", mitk::BoolProperty::New(true));
  if (planeGeometry)
  {
    auto segmentationNode = this->GetSegmentationImageNode();
    auto isContourPlaneGeometry = mitk::NodePredicateProperty::New("isContourPlaneGeometry", mitk::BoolProperty::New(true));

    mitk::DataStorage::SetOfObjects::ConstPointer contourNodes =
      m_DataStorage->GetDerivations(segmentationNode, isContourPlaneGeometry);

    auto contourFound = false;

    //  Go through the pre-existing contours and check if the contour position matches them.
    for (auto it = contourNodes->Begin(); it != contourNodes->End(); ++it)
    {
      auto layerID = dynamic_cast<mitk::UIntProperty *>(it->Value()->GetProperty("layerID"))->GetValue();
      auto labelID = dynamic_cast<mitk::UShortProperty *>(it->Value()->GetProperty("labelID"))->GetValue();
      auto posID = dynamic_cast<mitk::IntProperty *>(it->Value()->GetProperty("position"))->GetValue();
      bool sameLayer = (layerID == contourInfo.LayerValue);
      bool sameLabel = (labelID == contourInfo.LabelValue);
      bool samePos = (posID == contourInfo.Pos);

      if (samePos & sameLabel & sameLayer)
      {
        contourFound = true;
        it->Value()->SetData(planeGeometryData);
        break;
      }
    }

    if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
    {
      MITK_ERROR << "Invalid time point requested in AddPlaneGeometryNodeToDataStorage.";
      return;
    }

    const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

    //  Go through the contourPlaneGeometry Data and add the segmentationNode to it.
    if (!contourFound)
    {
      std::string contourName = "contourPlane " + std::to_string(m_ContourIndex);

      auto contourPlaneGeometryDataNode = mitk::DataNode::New();
      contourPlaneGeometryDataNode->SetData(planeGeometryData);

      //  No need to change properties
      contourPlaneGeometryDataNode->SetProperty("helper object", mitk::BoolProperty::New(false));
      contourPlaneGeometryDataNode->SetProperty("hidden object", mitk::BoolProperty::New(true));
      contourPlaneGeometryDataNode->SetProperty("isContourPlaneGeometry", mitk::BoolProperty::New(true));
      contourPlaneGeometryDataNode->SetVisibility(false);

      //  Need to change properties
      contourPlaneGeometryDataNode->SetProperty("name", mitk::StringProperty::New(contourName) );
      contourPlaneGeometryDataNode->SetProperty("layerID", mitk::UIntProperty::New(contourInfo.LayerValue));
      contourPlaneGeometryDataNode->SetProperty("labelID", mitk::UShortProperty::New(contourInfo.LabelValue));
      contourPlaneGeometryDataNode->SetProperty("position", mitk::IntProperty::New(contourInfo.Pos));
      contourPlaneGeometryDataNode->SetProperty("timeStep", mitk::IntProperty::New(currentTimeStep));

      contourPlaneGeometryDataNode->SetProperty("px", mitk::DoubleProperty::New(contourInfo.ContourPoint[0]));
      contourPlaneGeometryDataNode->SetProperty("py", mitk::DoubleProperty::New(contourInfo.ContourPoint[1]));
      contourPlaneGeometryDataNode->SetProperty("pz", mitk::DoubleProperty::New(contourInfo.ContourPoint[2]));

      m_DataStorage->Add(contourPlaneGeometryDataNode, segmentationNode);
    }
  }
}

void mitk::SurfaceInterpolationController::AddToInterpolationPipeline(ContourPositionInformation& contourInfo, bool reinitializationAction)
{
  if (!m_SelectedSegmentation)
    return;

  if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
    return;
  }

  //  Get current time step either from the
  auto GetCurrentTimeStep = [=](ContourPositionInformation contourInfo)
  {
    if (reinitializationAction)
    {
      return contourInfo.TimeStep;
    }
    return static_cast<size_t>(m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint));
  };
  const auto currentTimeStep = GetCurrentTimeStep(contourInfo);
  auto GetContourLayerID = [=](ContourPositionInformation contourInfo)
  {
    unsigned int currentLayerID;
    if(reinitializationAction)
    {
      if (contourInfo.LayerValue == std::numeric_limits<unsigned int>::max())
      {
        MITK_ERROR << "In mitk::SurfaceInterpolationController::AddToInterpolationPipeline. Problem in finding layerID";
      }
        currentLayerID = contourInfo.LayerValue;
    }
    else
    {
      try
      {
        currentLayerID = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation)->GetActiveLayer();
      }
      catch (const std::exception& e)
      {
        MITK_ERROR << "Unable to cast image to LabelSetImage. " << e.what() << '\n';
      }
    }
    return currentLayerID;
  };

  unsigned int currentLayerID = GetContourLayerID(contourInfo);

  ContourPositionInformationVec3D &currentImageContours = m_ListOfContours.at(m_SelectedSegmentation);
  ContourPositionInformationVec2D &currentTimeStepContoursList = currentImageContours.at(currentTimeStep);
  ContourPositionInformationList &currentContourList = currentTimeStepContoursList.at(currentLayerID);

  int replacementIndex = -1;
  int pos = -1;
  mitk::Surface* newContour = contourInfo.Contour;

  for (size_t i = 0; i < currentContourList.size(); i++)
  {
    auto& contourFromList = currentContourList.at(i);
    bool contoursAreCoplanar = ContoursCoplanar(contourInfo, contourFromList);
    bool contoursHaveSameLabel = contourInfo.LabelValue == contourFromList.LabelValue;

    //  Coplanar contours have the same "pos".
    if (contoursAreCoplanar)
    {
      pos = contourFromList.Pos;
      if (contoursHaveSameLabel)
      {
        replacementIndex = i;
      }
    }
  }
  //  The current contour has the same label and position as the current slice and a replacement is done.
  if (replacementIndex != -1)
  {
    contourInfo.Pos = pos;
    m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep).at(currentLayerID).at(replacementIndex) = contourInfo;

    if (!reinitializationAction)
    {
      this->AddPlaneGeometryNodeToDataStorage(contourInfo);
    }
    return;
  }

  //  Case that there is no contour in the current slice with the current label
  if (pos == -1)
    pos = m_ContourPosIndex++;

  m_ContourIndex++;
  contourInfo.Pos = pos;
  m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep).at(currentLayerID).push_back(contourInfo);

  if (contourInfo.Plane == nullptr)
  {
    MITK_ERROR << "contourInfo plane is null.";
  }
  if (!reinitializationAction)
  {
    this->AddPlaneGeometryNodeToDataStorage(contourInfo);
  }

  if (newContour->GetVtkPolyData()->GetNumberOfPoints() == 0)
  {
    this->RemoveContour(contourInfo);
    if  (m_ContourIndex > 0)
      m_ContourIndex--;
    if  (m_ContourIndex > 0)
      m_ContourIndex--;
  }
}

bool mitk::SurfaceInterpolationController::RemoveContour(ContourPositionInformation contourInfo)
{
  if (!m_SelectedSegmentation)
  {
    return false;
  }

  if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    return false;
  }

  const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);
  unsigned int currentLayerID = 0;
  try
  {
    currentLayerID = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation)->GetActiveLayer();
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what() << '\n';
  }

  auto it = m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep).at(currentLayerID).begin();
  while (it != m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep).at(currentLayerID).end())
  {
    const ContourPositionInformation &currentContour = (*it);
    if (ContoursCoplanar(currentContour, contourInfo))
    {
      m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep).at(currentLayerID).erase(it);
      this->ReinitializeInterpolation();
      return true;
    }
    ++it;
  }
  return false;
}

const mitk::Surface *mitk::SurfaceInterpolationController::GetContour(const ContourPositionInformation &contourInfo)
{
  if (!m_SelectedSegmentation)
  {
    return nullptr;
  }

  if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    return nullptr;
  }
  const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);
  const auto activeLayerID = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation)->GetActiveLayer();
  const auto &contourList = m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep).at(activeLayerID);

  for (auto &currentContour : contourList)
  {
    if (ContoursCoplanar(contourInfo, currentContour))
    {
      return currentContour.Contour;
    }
  }
  return nullptr;
}

unsigned int mitk::SurfaceInterpolationController::GetNumberOfContours()
{
  if (!m_SelectedSegmentation)
  {
    return -1;
  }

  if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    return -1;
  }
  const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);
  auto contourDoubleList = m_ListOfContours.at(m_SelectedSegmentation).at(currentTimeStep);

  unsigned int numContours = 0;
  for (auto& contourList : contourDoubleList)
  {

    numContours += contourList.size();
  }

  return numContours;
}

void mitk::SurfaceInterpolationController::AddActiveLabelContoursForInterpolation(mitk::Label::PixelType activeLabel)
{
  this->ReinitializeInterpolation();

  if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_ERROR << "Invalid time point requested for interpolation pipeline.";
    return;
  }
  const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

  unsigned int currentLayerID = 0;
  try
  {
    currentLayerID = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation)->GetActiveLayer();
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what() << '\n';
  }

  ContourPositionInformationVec3D &currentImageContours = m_ListOfContours.at(m_SelectedSegmentation);

  if (currentImageContours.size() <= currentTimeStep)
  {
    MITK_INFO << "Contours for current time step don't exist.";
    return;
  }
  ContourPositionInformationVec2D &currentTimeStepContoursList = currentImageContours.at(currentTimeStep);

  if (currentTimeStepContoursList.size() <= currentLayerID)
  {
    MITK_INFO << "Contours for current layer don't exist.";
    return;
  }
  ContourPositionInformationList &currentContours = currentTimeStepContoursList.at(currentLayerID);

  for (size_t i = 0; i < currentContours.size(); ++i)
  {
    if (currentContours.at(i).LabelValue == activeLabel)
    {
      m_ListOfInterpolationSessions.at(m_SelectedSegmentation).at(currentTimeStep).push_back(currentContours.at(i));
      m_ReduceFilter->SetInput(m_ListOfInterpolationSessions.at(m_SelectedSegmentation).at(currentTimeStep).size()-1, currentContours.at(i).Contour);
    }
  }
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_WARN << "No interpolation possible, currently selected timepoint is not in the time bounds of currently selected segmentation. Time point: " << m_CurrentTimePoint;
    m_InterpolationResult = nullptr;
    return;
  }
  const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);
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
  timeSelector->SetInput(m_SelectedSegmentation);
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
  interpolationResult->Expand(m_SelectedSegmentation->GetTimeSteps());

  auto geometry = m_SelectedSegmentation->GetTimeGeometry()->Clone();
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

mitk::Image::Pointer mitk::SurfaceInterpolationController::GetCurrentSegmentation()
{
  return m_SelectedSegmentation;
}

mitk::Image *mitk::SurfaceInterpolationController::GetImage()
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
  return m_ListOfInterpolationSessions.size();
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::SurfaceInterpolationController::GetImageBase(itk::Image<TPixel, VImageDimension> *input,
                                                        itk::ImageBase<3>::Pointer &result)
{
  result->Graft(input);
}

void mitk::SurfaceInterpolationController::SetCurrentSegmentationInterpolationList(mitk::Image::Pointer segmentation)
{
  this->SetCurrentInterpolationSession(segmentation);
}

void mitk::SurfaceInterpolationController::SetCurrentInterpolationSession(mitk::Image::Pointer currentSegmentationImage)
{
  if (currentSegmentationImage.GetPointer() == m_SelectedSegmentation)
  {
    return;
  }

  if (currentSegmentationImage.IsNull())
  {
    m_SelectedSegmentation = nullptr;
    return;
  }
  m_SelectedSegmentation = currentSegmentationImage.GetPointer();

  try
  {
    auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_SelectedSegmentation);
    auto it = m_ListOfContours.find(currentSegmentationImage.GetPointer());
    // If the session does not exist yet create a new ContourPositionPairList otherwise reinitialize the interpolation
    // pipeline
    if (it == m_ListOfContours.end())
    {
      ContourPositionInformationVec3D newList;

      auto numTimeSteps = labelSetImage->GetTimeGeometry()->CountTimeSteps();

      for (size_t t = 0; t < numTimeSteps; ++t)
      {
        auto twoDList = ContourPositionInformationVec2D();
        auto contourList = ContourPositionInformationList();
        twoDList.push_back(contourList);
        newList.push_back(twoDList);
      }

      m_ListOfContours[m_SelectedSegmentation] = newList;

      m_InterpolationResult = nullptr;
      m_CurrentNumberOfReducedContours = 0;

      auto command = itk::MemberCommand<SurfaceInterpolationController>::New();
      command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
      m_SegmentationObserverTags[m_SelectedSegmentation] = labelSetImage->AddObserver(itk::DeleteEvent(), command);

      m_NumberOfLayersInCurrentSegmentation = labelSetImage->GetNumberOfLayers();
    }

    // auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation);
    auto numLayersInSelectedSegmentation = labelSetImage->GetNumberOfLayers();
    //  Maybe this has to change.
    for (size_t layerID = 0; layerID < numLayersInSelectedSegmentation; ++layerID)
    {
      this->AddLabelSetConnection(layerID);
    }
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Unable to cast image as LabelSetImage";
  }

  auto it2 = m_ListOfInterpolationSessions.find(currentSegmentationImage.GetPointer());
  if (it2 == m_ListOfInterpolationSessions.end())
  {
    ContourPositionInformationVec2D newList;
    m_ListOfInterpolationSessions[m_SelectedSegmentation] = newList;
    m_InterpolationResult = nullptr;
    m_CurrentNumberOfReducedContours = 0;
  }

  this->ReinitializeInterpolation();
}

bool mitk::SurfaceInterpolationController::ReplaceInterpolationSession(mitk::Image::Pointer oldSession,
                                                                       mitk::Image::Pointer newSession)
{
  if (oldSession.IsNull() || newSession.IsNull())
    return false;

  if (oldSession.GetPointer() == newSession.GetPointer())
    return false;

  if (!mitk::Equal(*(oldSession->GetGeometry()), *(newSession->GetGeometry()), mitk::eps, false))
    return false;

  auto it = m_ListOfInterpolationSessions.find(oldSession.GetPointer());

  if (it == m_ListOfInterpolationSessions.end())
    return false;

  if (!newSession->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
  {
    MITK_WARN << "Interpolation session cannot be replaced. Currently selected timepoint is not in the time bounds of the new session. Time point: " << m_CurrentTimePoint;
    return false;
  }

  ContourPositionInformationVec2D oldList = (*it).second;

  m_ListOfInterpolationSessions[newSession.GetPointer()] = oldList;

  itk::MemberCommand<SurfaceInterpolationController>::Pointer command =
    itk::MemberCommand<SurfaceInterpolationController>::New();

  command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);

  m_SegmentationObserverTags[newSession] = newSession->AddObserver(itk::DeleteEvent(), command);

  if (m_SelectedSegmentation == oldSession)
    m_SelectedSegmentation = newSession;

  const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(m_SelectedSegmentation);
  timeSelector->SetTimeNr(currentTimeStep);
  timeSelector->SetChannelNr(0);
  timeSelector->Update();
  mitk::Image::Pointer refSegImage = timeSelector->GetOutput();

  m_NormalsFilter->SetSegmentationBinaryImage(refSegImage);

  this->RemoveInterpolationSession(oldSession);
  return true;
}

void mitk::SurfaceInterpolationController::RemoveSegmentationFromContourList(mitk::Image *segmentation)
{
  this->RemoveInterpolationSession(segmentation);
}

void mitk::SurfaceInterpolationController::RemoveInterpolationSession(mitk::Image::Pointer segmentationImage)
{
  if (segmentationImage)
  {
    if (m_SelectedSegmentation == segmentationImage)
    {
      m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
      m_SelectedSegmentation = nullptr;
    }
    m_ListOfInterpolationSessions.erase(segmentationImage);
    m_ListOfContours.erase(segmentationImage);

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
  // Removing all observers
  auto dataIter = m_SegmentationObserverTags.begin();
  while (dataIter != m_SegmentationObserverTags.end())
  {
    mitk::Image *image = (*dataIter).first;
    image->RemoveObserver((*dataIter).second);
    ++dataIter;
  }

  m_SegmentationObserverTags.clear();
  m_SelectedSegmentation = nullptr;
  m_ListOfInterpolationSessions.clear();
  m_ListOfContours.clear();
}


template <unsigned int VImageDimension = 3>
std::vector<mitk::Label::PixelType> GetPixelValuesPresentInImage(mitk::LabelSetImage* labelSetImage)
{
  mitk::ImagePixelReadAccessor<mitk::LabelSet::PixelType, VImageDimension> readAccessor(labelSetImage);
  std::vector<mitk::Label::PixelType> pixelsPresent;

  std::size_t numberOfPixels = 1;
  for (int dim = 0; dim < static_cast<int>(VImageDimension); ++dim)
    numberOfPixels *= static_cast<std::size_t>(readAccessor.GetDimension(dim));

  auto src = readAccessor.GetData();
  for (std::size_t i = 0; i < numberOfPixels; ++i)
  {
    mitk::Label::PixelType pixelVal = *(src + i);
    if ( (std::find(pixelsPresent.begin(), pixelsPresent.end(), pixelVal) == pixelsPresent.end()) && (pixelVal != 0) )
    {
      pixelsPresent.push_back(pixelVal);
    }
  }
  return pixelsPresent;
}

void mitk::SurfaceInterpolationController::RemoveContours(mitk::Label::PixelType label,
                                                          unsigned int timeStep,
                                                          unsigned int layerID)
{
  auto isContourEqualToLabelValue = [label] (ContourPositionInformation& contour) -> bool
  {
    return (contour.LabelValue == label);
  };

  ContourPositionInformationVec3D &currentImageContours = m_ListOfContours.at(m_SelectedSegmentation);
  ContourPositionInformationList &currentContourList = currentImageContours.at(timeStep).at(layerID);
  unsigned int numContoursBefore = currentContourList.size();
  auto it = std::remove_if(currentContourList.begin(), currentContourList.end(), isContourEqualToLabelValue);
  currentContourList.erase(it, currentContourList.end());
  unsigned int numContoursAfter = currentContourList.size();
  unsigned int numContours = numContoursAfter - numContoursBefore;
  m_ContourIndex -= numContours;
}

void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller,
                                                                 const itk::EventObject & /*event*/)
{
  auto *tempImage = dynamic_cast<mitk::Image *>(const_cast<itk::Object *>(caller));
  if (tempImage)
  {
    if (m_SelectedSegmentation == tempImage)
    {
      m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
      m_SelectedSegmentation = nullptr;
    }
    m_SegmentationObserverTags.erase(tempImage);
    m_ListOfContours.erase(tempImage);
    m_ListOfInterpolationSessions.erase(tempImage);
  }
}

void mitk::SurfaceInterpolationController::ReinitializeInterpolation()
{
  // If session has changed reset the pipeline
  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();

  //  Empty out the listOfInterpolationSessions
  m_ListOfInterpolationSessions[m_SelectedSegmentation].clear();

  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();

  if (m_SelectedSegmentation)
  {
    if (!m_SelectedSegmentation->GetTimeGeometry()->IsValidTimePoint(m_CurrentTimePoint))
    {
      MITK_WARN << "Interpolation cannot be reinitialized. Currently selected timepoint is not in the time bounds of the currently selected segmentation. Time point: " << m_CurrentTimePoint;
      return;
    }

    const auto currentTimeStep = m_SelectedSegmentation->GetTimeGeometry()->TimePointToTimeStep(m_CurrentTimePoint);

    //  Set reference image for interpolation surface filter
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(m_SelectedSegmentation);
    timeSelector->SetTimeNr(currentTimeStep);
    timeSelector->SetChannelNr(0);
    timeSelector->Update();
    mitk::Image::Pointer refSegImage = timeSelector->GetOutput();
    AccessFixedDimensionByItk_1(refSegImage, GetImageBase, 3, itkImage);
    m_InterpolateSurfaceFilter->SetReferenceImage(itkImage.GetPointer());

    //  Resize listofinterpolationsessions and listofcontours to numTimeSteps
    unsigned int numTimeSteps = m_SelectedSegmentation->GetTimeSteps();
    unsigned int size = m_ListOfInterpolationSessions[m_SelectedSegmentation].size();

    if (size != numTimeSteps)
    {
      m_ListOfInterpolationSessions.at(m_SelectedSegmentation).resize(numTimeSteps);
    }
  }
}

void mitk::SurfaceInterpolationController::AddLabelSetConnection(unsigned int layerID)
{
  if (m_SelectedSegmentation != nullptr)
  {
    try
    {
      auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation);
      auto previousLayerID = workingImage->GetActiveLayer();
      workingImage->SetActiveLayer(layerID);
      auto activeLabelSet = workingImage->GetLabelSet(layerID);

      if (activeLabelSet == nullptr)
        return;

      activeLabelSet->RemoveLabelEvent += mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
        this, &mitk::SurfaceInterpolationController::OnRemoveLabel);
      activeLabelSet->ActiveLabelEvent += mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
            this, &mitk::SurfaceInterpolationController::OnActiveLabel);
      workingImage->AfterChangeLayerEvent += mitk::MessageDelegate<mitk::SurfaceInterpolationController>(
        this, &mitk::SurfaceInterpolationController::OnLayerChanged);
        m_NumberOfConnectionsAdded += 1;
      workingImage->SetActiveLayer(previousLayerID);
    }
    catch(const std::exception& e)
    {
      MITK_ERROR << e.what() << '\n';
    }
  }
}

void mitk::SurfaceInterpolationController::AddLabelSetConnection()
{
  if (m_SelectedSegmentation != nullptr)
  {
    try
    {
      auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation);
      auto activeLabelSet = workingImage->GetActiveLabelSet();

      if (activeLabelSet == nullptr)
        return;

      activeLabelSet->RemoveLabelEvent += mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
        this, &mitk::SurfaceInterpolationController::OnRemoveLabel);
      workingImage->GetActiveLabelSet()->ActiveLabelEvent += mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
            this, &mitk::SurfaceInterpolationController::OnActiveLabel);
      workingImage->AfterChangeLayerEvent += mitk::MessageDelegate<mitk::SurfaceInterpolationController>(
        this, &mitk::SurfaceInterpolationController::OnLayerChanged);
        m_NumberOfConnectionsAdded += 1;
    }
    catch(const std::exception& e)
    {
      MITK_ERROR << e.what() << '\n';
    }
  }
}

void mitk::SurfaceInterpolationController::RemoveLabelSetConnection(mitk::LabelSetImage* labelSetImage, unsigned int layerID)
{
  labelSetImage->SetActiveLayer(layerID);
  labelSetImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
    this, &mitk::SurfaceInterpolationController::OnRemoveLabel);
  // labelSetImage->GetActiveLabelSet()->ActiveLabelEvent -= mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
  //       this, &mitk::SurfaceInterpolationController::OnActiveLabel);
  labelSetImage->AfterChangeLayerEvent -= mitk::MessageDelegate<mitk::SurfaceInterpolationController>(
    this, &mitk::SurfaceInterpolationController::OnLayerChanged);
  m_NumberOfConnectionsAdded -= 1;
}

void mitk::SurfaceInterpolationController::RemoveLabelSetConnection()
{
  if (m_SelectedSegmentation != nullptr)
  {
    try
    {
      auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation);
      workingImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
        this, &mitk::SurfaceInterpolationController::OnRemoveLabel);
      workingImage->GetActiveLabelSet()->ActiveLabelEvent -= mitk::MessageDelegate1<mitk::SurfaceInterpolationController, mitk::Label::PixelType>(
            this, &mitk::SurfaceInterpolationController::OnActiveLabel);
      workingImage->AfterChangeLayerEvent -= mitk::MessageDelegate<mitk::SurfaceInterpolationController>(
        this, &mitk::SurfaceInterpolationController::OnLayerChanged);
    }
    catch (const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
  }
}

void mitk::SurfaceInterpolationController::OnRemoveLabel(mitk::Label::PixelType /*removedLabelValue*/)
{
  if (m_SelectedSegmentation != nullptr)
  {
    auto numTimeSteps = m_SelectedSegmentation->GetTimeGeometry()->CountTimeSteps();
    try
    {
      auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_SelectedSegmentation);
      auto currentLayerID = labelSetImage->GetActiveLayer();

      for(unsigned int t = 0; t < numTimeSteps; ++t)
      {
        this->RemoveContours(m_PreviousActiveLabelValue,t,currentLayerID);
      }

    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
  }
}

void mitk::SurfaceInterpolationController::OnActiveLabel(mitk::Label::PixelType newActiveLabelValue)
{
  m_PreviousActiveLabelValue = m_CurrentActiveLabelValue;
  m_CurrentActiveLabelValue = newActiveLabelValue;
}

unsigned int mitk::SurfaceInterpolationController::GetNumberOfLayersInCurrentSegmentation() const
{
  return m_NumberOfLayersInCurrentSegmentation;
}

void mitk::SurfaceInterpolationController::SetNumberOfLayersInCurrentSegmentation(unsigned int numLayers)
{
  m_NumberOfLayersInCurrentSegmentation = numLayers;
}

void mitk::SurfaceInterpolationController::OnAddLayer()
{
  assert(m_SelectedSegmentation != nullptr);
  auto& contoursForSegmentation = m_ListOfContours.at(m_SelectedSegmentation);
  //  Push an information list for each time step.
  for(size_t t = 0; t < contoursForSegmentation.size(); ++t)
  {
    contoursForSegmentation.at(t).push_back( ContourPositionInformationList() );
  }
}

void mitk::SurfaceInterpolationController::OnRemoveLayer()
{
  assert(m_SelectedSegmentation != nullptr);
  auto& contoursForSegmentation = m_ListOfContours.at(m_SelectedSegmentation);
  //  Erase the layers in each of the time steps.

  //  The previous layer is removed
  for (size_t t = 0; t < contoursForSegmentation.size(); ++t)
  {
    assert(m_PreviousLayerIndex < contoursForSegmentation.at(t).size());
    auto& contoursAtTimeStep = contoursForSegmentation.at(t);
    for (size_t c = m_CurrentLayerIndex+1; c < contoursAtTimeStep.size(); ++c)
    {
      auto& contoursInCurrentLayer = contoursAtTimeStep.at(c);
      for (auto& contour : contoursInCurrentLayer)
      {
        contour.LayerValue = contour.LayerValue - 1;
      }
    }
  }

  for (size_t t = 0; t < contoursForSegmentation.size(); ++t)
  {
    assert (m_CurrentLayerIndex < contoursForSegmentation.at(t).size());
    contoursForSegmentation.at(t).erase(contoursForSegmentation.at(t).begin() + m_PreviousLayerIndex);
  }
  this->Modified();
}

void mitk::SurfaceInterpolationController::OnLayerChanged()
{
  auto currentLayer = dynamic_cast<mitk::LabelSetImage*>(m_SelectedSegmentation)->GetActiveLayer();
  m_PreviousLayerIndex = m_CurrentLayerIndex;
  m_CurrentLayerIndex = currentLayer;
}

mitk::SurfaceInterpolationController::ContourPositionInformationList* mitk::SurfaceInterpolationController::GetContours(unsigned int timeStep, unsigned int layerID)
{
  if (m_SelectedSegmentation == nullptr)
    return nullptr;

  if (timeStep >= m_ListOfContours.at(m_SelectedSegmentation).size())
    return nullptr;

  if (layerID >= m_ListOfContours.at(m_SelectedSegmentation).at(timeStep).size())
    return nullptr;

  return &m_ListOfContours[m_SelectedSegmentation][timeStep][layerID];
}

void mitk::SurfaceInterpolationController::CompleteReinitialization(const std::vector<mitk::Surface::Pointer>& contourList,
                                                                    std::vector<const mitk::PlaneGeometry *>& contourPlanes)
{
  this->ClearInterpolationSession();

  auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_SelectedSegmentation);
  auto numLayers = labelSetImage->GetNumberOfLayers();

  //  Add layers to the m_ListOfContours
  for (size_t layer = 0; layer < numLayers; ++layer)
  {
    this->OnAddLayer();
  }

  //  Now the layers should be empty and the new layers can be added.
  this->AddNewContours(contourList, contourPlanes, true);
}

void mitk::SurfaceInterpolationController::ClearInterpolationSession()
{
  if (m_SelectedSegmentation != nullptr)
  {
    auto it = m_ListOfContours.find(m_SelectedSegmentation);
    if (it != m_ListOfContours.end())
    {
      auto timeSteps = m_ListOfContours[m_SelectedSegmentation].size();
      try
      {
        auto labelSetImage = dynamic_cast<mitk::LabelSetImage *>(m_SelectedSegmentation);
        auto labelSetImageTimeSteps = labelSetImage->GetTimeGeometry()->CountTimeSteps();

        if (timeSteps != labelSetImageTimeSteps)
        {
          MITK_ERROR << "Time steps are not the same.";
        }

        for (size_t t = 0; t < timeSteps; ++t)
        {
          m_ListOfContours[m_SelectedSegmentation][t].clear();
        }

      }
      catch(std::bad_cast& e)
      {
        MITK_ERROR << "Unable to cast m_SelectedSegmentation to labelSetImage in ClearInterpolationSession";
      }
    }
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
  mitk::ImagePixelReadAccessor<mitk::LabelSet::PixelType, VImageDimension> readAccessor(labelSetImage);

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
