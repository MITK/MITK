/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSurfaceInterpolationController.h>

#include <shared_mutex>

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
#include <mitkNodePredicateAnd.h>
#include <mitkPlanarCircle.h>
#include <mitkPlaneGeometry.h>
#include <mitkReduceContourSetFilter.h>

#include <vtkFieldData.h>
#include <vtkMath.h>
#include <vtkPolygon.h>


typedef std::map<mitk::TimeStepType, mitk::SurfaceInterpolationController::CPIVector> CPITimeStepMap;
typedef std::map<mitk::LabelSetImage::LabelValueType, CPITimeStepMap> CPITimeStepLabelMap;

typedef std::map<const mitk::LabelSetImage*, CPITimeStepLabelMap> CPITimeStepLabelSegMap;

CPITimeStepLabelSegMap cpiMap;
std::shared_mutex cpiMutex;

std::map<mitk::LabelSetImage*, unsigned long> segmentationObserverTags;

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
  : m_SelectedSegmentation(nullptr),
    m_CurrentTimePoint(0.)
{
  m_DistanceImageSpacing = 0.0;
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  m_ReduceFilter->SetUseProgressBar(false);
  m_NormalsFilter->SetUseProgressBar(true);
  m_NormalsFilter->SetProgressStepSize(1);
  m_InterpolateSurfaceFilter->SetUseProgressBar(true);
  m_InterpolateSurfaceFilter->SetProgressStepSize(7);

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
  while (segmentationObserverTags.size())
  {
    this->RemoveObserversInternal(segmentationObserverTags.begin()->first);
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

void mitk::SurfaceInterpolationController::AddNewContours(const std::vector<ContourPositionInformation>& newCPIs,
  bool reinitializationAction)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull()) return;

  for (auto cpi : newCPIs)
  {
    if (cpi.Contour->GetVtkPolyData()->GetNumberOfPoints() > 0)
    {
      this->AddToInterpolationPipeline(cpi, reinitializationAction);
    }
  }
  this->Modified();
}


mitk::DataNode* GetSegmentationImageNodeInternal(mitk::DataStorage* ds, mitk::LabelSetImage* seg)
{
  if (nullptr == ds) return nullptr;
  if (nullptr == seg) return nullptr;

  mitk::DataNode* segmentationNode = nullptr;
  mitk::NodePredicateDataUID::Pointer dataUIDPredicate = mitk::NodePredicateDataUID::New(seg->GetUID());
  auto dataNodeObjects = ds->GetSubset(dataUIDPredicate);

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

mitk::DataNode* mitk::SurfaceInterpolationController::GetSegmentationImageNode() const
{
  if (m_DataStorage.IsNull()) return nullptr;
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull()) return nullptr;
  return GetSegmentationImageNodeInternal(this->m_DataStorage, selectedSegmentation);
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::SurfaceInterpolationController::GetPlaneGeometryNodeFromDataStorage(const DataNode* segNode, LabelSetImage::LabelValueType labelValue, TimeStepType timeStep) const
{
  DataStorage::SetOfObjects::Pointer relevantNodes = DataStorage::SetOfObjects::New();

  if (m_DataStorage.IsNotNull())
  {
    //remove relevant plane nodes
    auto nodes = this->GetPlaneGeometryNodeFromDataStorage(segNode, labelValue);

    for (auto it = nodes->Begin(); it != nodes->End(); ++it)
    {
      auto aTS = dynamic_cast<mitk::IntProperty*>(it->Value()->GetProperty("timeStep"))->GetValue();
      bool sameTS = (timeStep == aTS);

      if (sameTS)
      {
        relevantNodes->push_back(it->Value());
      }
    }
  }
  return relevantNodes;
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::SurfaceInterpolationController::GetPlaneGeometryNodeFromDataStorage(const DataNode* segNode, LabelSetImage::LabelValueType labelValue) const
{
  auto isContourPlaneGeometry = NodePredicateProperty::New("isContourPlaneGeometry", mitk::BoolProperty::New(true));
  auto isCorrectLabel = NodePredicateProperty::New("labelID", mitk::UShortProperty::New(labelValue));
  auto searchPredicate = NodePredicateAnd::New(isContourPlaneGeometry, isCorrectLabel);

  mitk::DataStorage::SetOfObjects::ConstPointer result;
  if (m_DataStorage.IsNotNull()) result = m_DataStorage->GetDerivations(segNode, searchPredicate);
  return result;
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::SurfaceInterpolationController::GetPlaneGeometryNodeFromDataStorage(const DataNode* segNode) const
{
  auto isContourPlaneGeometry = NodePredicateProperty::New("isContourPlaneGeometry", mitk::BoolProperty::New(true));

  mitk::DataStorage::SetOfObjects::ConstPointer result;
  if (m_DataStorage.IsNotNull()) result = m_DataStorage->GetDerivations(segNode, isContourPlaneGeometry);
  return result;
}
void mitk::SurfaceInterpolationController::AddPlaneGeometryNodeToDataStorage(const ContourPositionInformation& contourInfo) const
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

  if (m_DataStorage.IsNull())
  {
    MITK_DEBUG << "Cannot add plane geometry nodes. No data storage is set.";
    return;
  }

  auto planeGeometry = contourInfo.Plane;
  if (planeGeometry)
  {
    auto segmentationNode = this->GetSegmentationImageNode();
    mitk::DataStorage::SetOfObjects::ConstPointer contourNodes = this->GetPlaneGeometryNodeFromDataStorage(segmentationNode, contourInfo.LabelValue, contourInfo.TimeStep);

    mitk::DataNode::Pointer contourPlaneGeometryDataNode;

    //  Go through the pre-existing contours and check if the contour position matches them.
    for (auto it = contourNodes->Begin(); it != contourNodes->End(); ++it)
    {
      auto planeData = dynamic_cast<mitk::PlanarFigure*>(it->Value()->GetData());
      if (nullptr == planeData) mitkThrow() << "Invalid ContourPlaneGeometry data node. Does not contion a planar figure as data.";

      bool samePlane = contourInfo.Plane->IsOnPlane(planeData->GetPlaneGeometry());

      if (samePlane)
      {
        contourPlaneGeometryDataNode = it->Value();
        break;
      }
    }

    //  Go through the contourPlaneGeometry Data and add the segmentationNode to it.
    if (contourPlaneGeometryDataNode.IsNull())
    {
      auto planeGeometryData = mitk::PlanarCircle::New();
      planeGeometryData->SetPlaneGeometry(planeGeometry->Clone());
      mitk::Point2D p1;
      planeGeometry->Map(planeGeometry->GetCenter(), p1);
      planeGeometryData->PlaceFigure(p1);
      planeGeometryData->SetCurrentControlPoint(p1);
      planeGeometryData->SetProperty("initiallyplaced", mitk::BoolProperty::New(true));

      std::string contourName = "contourPlane L " + std::to_string(contourInfo.LabelValue) + " T " + std::to_string(contourInfo.TimeStep);

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
      contourPlaneGeometryDataNode->SetProperty("timeStep", mitk::IntProperty::New(contourInfo.TimeStep));

      contourPlaneGeometryDataNode->SetData(planeGeometryData);

      m_DataStorage->Add(contourPlaneGeometryDataNode, segmentationNode);
    }
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
    std::lock_guard<std::shared_mutex> guard(cpiMutex);
    const auto& currentTimeStep = contourInfo.TimeStep;
    const auto& currentLabelValue = contourInfo.LabelValue;

    auto& currentImageContours = cpiMap[selectedSegmentation];
    auto& currentLabelContours = currentImageContours[currentLabelValue];
    auto& currentContourList = currentLabelContours[currentTimeStep];

    auto finding = std::find_if(currentContourList.begin(), currentContourList.end(), [contourInfo](const ContourPositionInformation& element) {return contourInfo.Plane->IsOnPlane(element.Plane); });

    if (finding != currentContourList.end())
    {
      MITK_DEBUG << "CPI already exists. CPI is updated. Label: "<< currentLabelValue << "; Time Step: " << currentTimeStep;
      *finding = contourInfo;
    }
    else
    {
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
    std::lock_guard<std::shared_mutex> cpiGuard(cpiMutex);

    const auto currentTimeStep = contourInfo.TimeStep;
    const auto currentLabel = contourInfo.LabelValue;
    auto it = cpiMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep).begin();


    while (it != cpiMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep).end())
    {
      const ContourPositionInformation& currentContour = (*it);
      if (currentContour.Plane->IsOnPlane(contourInfo.Plane))
      {
        cpiMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep).erase(it);
        removedIt = true;

        if (m_DataStorage.IsNotNull())
        {
          mitk::DataNode::Pointer contourPlaneGeometryDataNode;

          auto contourNodes = this->GetPlaneGeometryNodeFromDataStorage(GetSegmentationImageNodeInternal(m_DataStorage, selectedSegmentation), currentLabel, currentTimeStep);

          //  Go through the nodes and check if the contour position matches them.
          for (auto it = contourNodes->Begin(); it != contourNodes->End(); ++it)
          {
            auto planeData = dynamic_cast<mitk::PlanarFigure*>(it->Value()->GetData());
            if (nullptr == planeData) mitkThrow() << "Invalid ContourPlaneGeometry data node. Does not contion a planar figure as data.";

            bool samePlane = contourInfo.Plane->IsOnPlane(planeData->GetPlaneGeometry());

            if (samePlane)
            {
              m_DataStorage->Remove(it->Value());
              break;
            }
          }
        }
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

  std::shared_lock<std::shared_mutex> guard(cpiMutex);

  auto& currentImageContours = cpiMap.at(selectedSegmentation);

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

  // Last progress step
  mitk::ProgressBar::GetInstance()->Progress(20);
  m_InterpolationResult->DisconnectPipeline();
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::GetInterpolationResult()
{
  return m_InterpolationResult;
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

unsigned int mitk::SurfaceInterpolationController::GetNumberOfInterpolationSessions()
{
  return cpiMap.size();
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

  m_SelectedSegmentation = currentSegmentationImage;
  selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation.IsNotNull())
  {
    std::lock_guard<std::shared_mutex> guard(cpiMutex);

    auto it = cpiMap.find(selectedSegmentation);
    if (it == cpiMap.end())
    {
      cpiMap[selectedSegmentation] = CPITimeStepLabelMap();

      auto command = itk::MemberCommand<SurfaceInterpolationController>::New();
      command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
      segmentationObserverTags[selectedSegmentation] = selectedSegmentation->AddObserver(itk::DeleteEvent(), command);
      selectedSegmentation->AddLabelRemovedListener(mitk::MessageDelegate1<SurfaceInterpolationController, mitk::LabelSetImage::LabelValueType>(
        this, &SurfaceInterpolationController::OnRemoveLabel));
    }

  }

  m_InterpolationResult = nullptr;
  m_CurrentNumberOfReducedContours = 0;
  m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
  this->ReinitializeInterpolation();
}

void mitk::SurfaceInterpolationController::RemoveInterpolationSession(mitk::LabelSetImage* segmentationImage)
{
  if (nullptr != segmentationImage)
  {
    auto selectedSegmentation = m_SelectedSegmentation.Lock();
    if (selectedSegmentation == segmentationImage)
    {
      this->SetCurrentInterpolationSession(nullptr);
    }

    {
      std::lock_guard<std::shared_mutex> guard(cpiMutex);
      this->RemoveObserversInternal(segmentationImage);
      cpiMap.erase(segmentationImage);
      if (m_DataStorage.IsNotNull())
      {
        auto nodes = this->GetPlaneGeometryNodeFromDataStorage(GetSegmentationImageNodeInternal(this->m_DataStorage, segmentationImage));
        this->m_DataStorage->Remove(nodes);
      }
    }

  }
}

void mitk::SurfaceInterpolationController::RemoveObserversInternal(mitk::LabelSetImage* segmentationImage)
{
  auto pos = segmentationObserverTags.find(segmentationImage);
  if (pos != segmentationObserverTags.end())
  {
    segmentationImage->RemoveObserver((*pos).second);
    segmentationImage->RemoveLabelRemovedListener(mitk::MessageDelegate1<SurfaceInterpolationController, mitk::LabelSetImage::LabelValueType>(
      this, &SurfaceInterpolationController::OnRemoveLabel));
    segmentationObserverTags.erase(segmentationImage);
  }
}

void mitk::SurfaceInterpolationController::RemoveAllInterpolationSessions()
{
  this->RemoveObservers();
  m_NormalsFilter->SetSegmentationBinaryImage(nullptr);
  m_SelectedSegmentation = nullptr;

  std::lock_guard<std::shared_mutex> guard(cpiMutex);
  cpiMap.clear();
}

void mitk::SurfaceInterpolationController::RemoveContours(mitk::Label::PixelType label,
  TimeStepType timeStep)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow() << "Cannot remove contours. No valid segmentation selected.";
  }

  std::lock_guard<std::shared_mutex> guard(cpiMutex);

  auto& cpiLabelMap = cpiMap[selectedSegmentation];
  auto finding = cpiLabelMap.find(label);

  if (finding != cpiLabelMap.end())
  {
    finding->second.erase(timeStep);
  }

  if (m_DataStorage.IsNotNull())
  {
    //remove relevant plane nodes
    auto nodes = this->GetPlaneGeometryNodeFromDataStorage(GetSegmentationImageNodeInternal(this->m_DataStorage, selectedSegmentation), label, timeStep);

    this->m_DataStorage->Remove(nodes);
  }
}

void mitk::SurfaceInterpolationController::RemoveContours(mitk::Label::PixelType label)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    mitkThrow() << "Cannot remove contours. No valid segmentation selected.";
  }

  std::lock_guard<std::shared_mutex> guard(cpiMutex);
  cpiMap[selectedSegmentation].erase(label);

  if (m_DataStorage.IsNotNull())
  {
    //remove relevant plane nodes
    auto nodes = this->GetPlaneGeometryNodeFromDataStorage(GetSegmentationImageNodeInternal(this->m_DataStorage, selectedSegmentation), label);
    this->m_DataStorage->Remove(nodes);
  }
}


void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller,
                                                                 const itk::EventObject & /*event*/)
{
  auto tempImage = dynamic_cast<mitk::LabelSetImage *>(const_cast<itk::Object *>(caller));
  if (tempImage)
  {
    this->RemoveInterpolationSession(tempImage);
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

mitk::SurfaceInterpolationController::CPIVector* mitk::SurfaceInterpolationController::GetContours(LabelSetImage::LabelValueType labelValue, TimeStepType timeStep)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation == nullptr)
    return nullptr;

  std::shared_lock<std::shared_mutex> guard(cpiMutex);

  auto labelFinding = cpiMap[selectedSegmentation].find(labelValue);

  if (labelFinding != cpiMap[selectedSegmentation].end())
  {
    auto tsFinding = labelFinding->second.find(timeStep);

    if (tsFinding != labelFinding->second.end())
    {
      return &(tsFinding->second);
    }
  }

  return nullptr;
}

std::vector<mitk::LabelSetImage::LabelValueType> mitk::SurfaceInterpolationController::GetAffectedLabels(const LabelSetImage* seg, TimeStepType timeStep, const PlaneGeometry* plane) const
{
  std::lock_guard<std::shared_mutex> guard(cpiMutex);

  std::vector<mitk::LabelSetImage::LabelValueType> result;

  auto finding = cpiMap.find(seg);
  if (finding == cpiMap.end()) return result;
  auto currentImageContours = finding->second;

  for (auto [label, contours] : currentImageContours)
  {
    auto tsFinding = contours.find(timeStep);
    if (tsFinding != contours.end())
    {
      auto cpis = tsFinding->second;
      auto finding = std::find_if(cpis.begin(), cpis.end(), [plane](const ContourPositionInformation& element) {return plane->IsOnPlane(element.Plane); });

      if (finding != cpis.end())
      {
        result.push_back(label);
      }
    }
  }
  return result;
}


void mitk::SurfaceInterpolationController::CompleteReinitialization(const std::vector<ContourPositionInformation>& newCPIs)
{
  this->ClearInterpolationSession();

  //  Now the layers should be empty and the new layers can be added.
  this->AddNewContours(newCPIs, true);
}

void mitk::SurfaceInterpolationController::ClearInterpolationSession()
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();

  if (selectedSegmentation != nullptr)
  {
    std::lock_guard<std::shared_mutex> guard(cpiMutex);
    cpiMap[selectedSegmentation].clear();
  }
}
