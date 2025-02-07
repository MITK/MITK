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
#include <mitkImageTimeSelector.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkNodePredicateDataUID.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkPlanarCircle.h>
#include <mitkPlaneGeometry.h>
#include <mitkReduceContourSetFilter.h>

struct CPICache
{
  mitk::SurfaceInterpolationController::CPIVector cpis;
  itk::TimeStamp cpiTimeStamp;
  mitk::Surface::Pointer cachedSurface;
};

typedef std::map<mitk::TimeStepType, CPICache> CPITimeStepMap;
typedef std::map<mitk::MultiLabelSegmentation::LabelValueType, CPITimeStepMap> CPITimeStepLabelMap;

typedef std::map<const mitk::MultiLabelSegmentation*, CPITimeStepLabelMap> CPITimeStepLabelSegMap;

CPITimeStepLabelSegMap cpiMap;
std::shared_mutex cpiMutex;

std::map<mitk::MultiLabelSegmentation*, unsigned long> segmentationObserverTags;
std::map<mitk::MultiLabelSegmentation*, unsigned long> labelRemovedObserverTags;

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
  : m_DistanceImageVolume(50000),
    m_SelectedSegmentation(nullptr)
{
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
  bool reinitializationAction, bool silent)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull()) return;

  for (auto cpi : newCPIs)
  {
    if (cpi.Contour->GetVtkPolyData()->GetNumberOfPoints() > 0)
    {
      this->AddToCPIMap(cpi, reinitializationAction);
    }
  }
  if (!silent) this->Modified();
}


mitk::DataNode* GetSegmentationImageNodeInternal(mitk::DataStorage* ds, const mitk::MultiLabelSegmentation* seg)
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

mitk::DataStorage::SetOfObjects::ConstPointer mitk::SurfaceInterpolationController::GetPlaneGeometryNodeFromDataStorage(const DataNode* segNode, MultiLabelSegmentation::LabelValueType labelValue, TimeStepType timeStep) const
{
  DataStorage::SetOfObjects::Pointer relevantNodes = DataStorage::SetOfObjects::New();

  if (m_DataStorage.IsNotNull())
  {
    //remove relevant plane nodes
    auto nodes = this->GetPlaneGeometryNodeFromDataStorage(segNode, labelValue);

    for (auto it = nodes->Begin(); it != nodes->End(); ++it)
    {
      auto aTS = static_cast<TimeStepType>(dynamic_cast<mitk::IntProperty*>(it->Value()->GetProperty("timeStep"))->GetValue());
      bool sameTS = (timeStep == aTS);

      if (sameTS)
      {
        relevantNodes->push_back(it->Value());
      }
    }
  }
  return relevantNodes;
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::SurfaceInterpolationController::GetPlaneGeometryNodeFromDataStorage(const DataNode* segNode, MultiLabelSegmentation::LabelValueType labelValue) const
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

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimeStep(contourInfo.TimeStep))
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

void mitk::SurfaceInterpolationController::AddToCPIMap(ContourPositionInformation& contourInfo, bool reinitializationAction)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
    return;

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimeStep(contourInfo.TimeStep))
  {
    MITK_ERROR << "Invalid time step requested for interpolation pipeline.";
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
    auto& currentCPICache = currentLabelContours[currentTimeStep];
    auto& currentContourList = currentCPICache.cpis;

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
    currentCPICache.cpiTimeStamp.Modified();
  }

  if (!reinitializationAction)
  {
    this->AddPlaneGeometryNodeToDataStorage(contourInfo);
  }
}

bool mitk::SurfaceInterpolationController::RemoveContour(ContourPositionInformation contourInfo, bool keepPlaceholderForUndo)
{
  auto selectedSegmentation = m_SelectedSegmentation.Lock();
  if (selectedSegmentation.IsNull())
  {
    return false;
  }

  if (!selectedSegmentation->GetTimeGeometry()->IsValidTimeStep(contourInfo.TimeStep))
  {
    return false;
  }

  bool removedIt = false;

  {
    std::lock_guard<std::shared_mutex> cpiGuard(cpiMutex);

    const auto currentTimeStep = contourInfo.TimeStep;
    const auto currentLabel = contourInfo.LabelValue;
    auto& cpiCache = cpiMap.at(selectedSegmentation).at(currentLabel).at(currentTimeStep);
    auto it = cpiCache.cpis.begin();


    while (it != cpiCache.cpis.end())
    {
      const ContourPositionInformation& currentContour = (*it);
      if (currentContour.Plane->IsOnPlane(contourInfo.Plane))
      {
        if (keepPlaceholderForUndo)
        {
          it->Contour = nullptr;
        }
        else
        {
          cpiCache.cpis.erase(it);
        }
        cpiCache.cpiTimeStamp.Modified();
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

  return removedIt;
}

void mitk::SurfaceInterpolationController::AddActiveLabelContoursForInterpolation(ReduceContourSetFilter* reduceFilter, const MultiLabelSegmentation* segmentationImage, MultiLabelSegmentation::LabelValueType labelValue, TimeStepType timeStep)
{
  const auto& currentImageContours = cpiMap.at(segmentationImage);

  auto finding = currentImageContours.find(labelValue);
  if (finding == currentImageContours.end())
  {
    MITK_INFO << "Contours for label don't exist. Label value: " << labelValue;
    return;
  }

  const auto& currentLabelContoursMap = finding->second;

  auto tsfinding = currentLabelContoursMap.find(timeStep);
  if (tsfinding == currentLabelContoursMap.end())
  {
    MITK_INFO << "Contours for current time step don't exist.";
    return;
  }

  const auto& currentContours = tsfinding->second.cpis;

  unsigned int index = 0;
  for (const auto&  cpi : currentContours)
  {
    if (!cpi.IsPlaceHolder())
    {
      reduceFilter->SetInput(index, cpi.Contour);
      ++index;
    }
  }
}

bool CPICacheIsOutdated(const mitk::MultiLabelSegmentation* segmentationImage, mitk::MultiLabelSegmentation::LabelValueType labelValue, mitk::TimeStepType timeStep)
{
  const auto& currentImageContours = cpiMap.at(segmentationImage);

  auto finding = currentImageContours.find(labelValue);
  if (finding == currentImageContours.end())
  {
    return false;
  }

  const auto& currentLabelContoursMap = finding->second;

  auto tsfinding = currentLabelContoursMap.find(timeStep);
  if (tsfinding == currentLabelContoursMap.end())
  {
    return false;
  }

  bool result = tsfinding->second.cachedSurface.IsNull() || tsfinding->second.cachedSurface->GetMTime() < tsfinding->second.cpiTimeStamp.GetMTime();
  return result;
}

void SetCPICacheSurface(mitk::Surface* surface, const mitk::MultiLabelSegmentation* segmentationImage, mitk::MultiLabelSegmentation::LabelValueType labelValue, mitk::TimeStepType timeStep)
{
  const auto& currentImageContours = cpiMap.at(segmentationImage);

  auto finding = currentImageContours.find(labelValue);
  if (finding == currentImageContours.end())
  {
    return;
  }

  const auto& currentLabelContoursMap = finding->second;

  auto tsfinding = currentLabelContoursMap.find(timeStep);
  if (tsfinding == currentLabelContoursMap.end())
  {
    return;
  }

  cpiMap[segmentationImage][labelValue][timeStep].cachedSurface = surface;
}

void mitk::SurfaceInterpolationController::Interpolate(const MultiLabelSegmentation* segmentationImage, MultiLabelSegmentation::LabelValueType labelValue, TimeStepType timeStep)
{
  if (nullptr == segmentationImage)
  {
    mitkThrow() << "Cannot interpolate contours. No valid segmentation passed.";
  }

  std::lock_guard<std::shared_mutex> guard(cpiMutex);
  auto it = cpiMap.find(segmentationImage);
  if (it == cpiMap.end())
  {
    mitkThrow() << "Cannot interpolate contours. Passed segmentation is not registered at controller.";
  }

  if (!segmentationImage->ExistLabel(labelValue))
  {
    mitkThrow() << "Cannot interpolate contours. None existent label request. Invalid label:" << labelValue;
  }

  if (!segmentationImage->GetTimeGeometry()->IsValidTimeStep(timeStep))
  {
    mitkThrow() << "Cannot interpolate contours. No valid time step requested. Invalid time step:" << timeStep;
  }

  if (!CPICacheIsOutdated(segmentationImage, labelValue, timeStep)) return;

  mitk::Surface::Pointer interpolationResult = nullptr;

  auto reduceFilter = ReduceContourSetFilter::New();
  auto normalsFilter = ComputeContourSetNormalsFilter::New();
  auto interpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  const auto spacing = segmentationImage->GetGeometry(timeStep)->GetSpacing();
  double minSpacing = 100;
  double maxSpacing = 0;
  for (int i = 0; i < 3; i++)
  {
    if (spacing[i] < minSpacing)
    {
      minSpacing = spacing[i];
    }
    if (spacing[i] > maxSpacing)
    {
      maxSpacing = spacing[i];
    }
  }
  reduceFilter->SetMinSpacing(minSpacing);
  reduceFilter->SetMaxSpacing(maxSpacing);
  normalsFilter->SetMaxSpacing(maxSpacing);
  interpolateSurfaceFilter->SetDistanceImageVolume(m_DistanceImageVolume);

  reduceFilter->SetUseProgressBar(false);
  normalsFilter->SetUseProgressBar(true);
  normalsFilter->SetProgressStepSize(1);
  interpolateSurfaceFilter->SetUseProgressBar(true);
  interpolateSurfaceFilter->SetProgressStepSize(7);

  //  Set reference image for interpolation surface filter
  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(segmentationImage);
  timeSelector->SetTimeNr(timeStep);
  timeSelector->SetChannelNr(0);
  timeSelector->Update();
  mitk::Image::Pointer refSegImage = timeSelector->GetOutput();
  AccessFixedDimensionByItk_1(refSegImage, GetImageBase, 3, itkImage);
  interpolateSurfaceFilter->SetReferenceImage(itkImage.GetPointer());

  try
  {
    this->AddActiveLabelContoursForInterpolation(reduceFilter, segmentationImage, labelValue, timeStep);
    reduceFilter->Update();
    auto currentNumberOfReducedContours = reduceFilter->GetNumberOfOutputs();

    if (currentNumberOfReducedContours < 2)
    {
      // If no interpolation is possible reset the interpolation result
      MITK_INFO << "Interpolation impossible: not enough contours.";
    }
    else
    {
      normalsFilter->SetSegmentationBinaryImage(refSegImage);

      for (size_t i = 0; i < currentNumberOfReducedContours; ++i)
      {
        mitk::Surface::Pointer reducedContour = reduceFilter->GetOutput(i);
        reducedContour->DisconnectPipeline();
        normalsFilter->SetInput(i, reducedContour);
        interpolateSurfaceFilter->SetInput(i, normalsFilter->GetOutput(i));
      }

      // Setting up progress bar
      mitk::ProgressBar::GetInstance()->AddStepsToDo(10);

      // create a surface from the distance-image
      auto imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
      imageToSurfaceFilter->SetInput(interpolateSurfaceFilter->GetOutput());
      imageToSurfaceFilter->SetThreshold(0);
      imageToSurfaceFilter->SetSmooth(true);
      imageToSurfaceFilter->SetSmoothIteration(1);
      imageToSurfaceFilter->Update();

      interpolationResult = mitk::Surface::New();
      interpolationResult->Expand(segmentationImage->GetTimeSteps());

      auto geometry = segmentationImage->GetTimeGeometry()->Clone();
      geometry->ReplaceTimeStepGeometries(mitk::Geometry3D::New());
      interpolationResult->SetTimeGeometry(geometry);

      interpolationResult->SetVtkPolyData(imageToSurfaceFilter->GetOutput()->GetVtkPolyData(), timeStep);
      interpolationResult->DisconnectPipeline();

      // Last progress step
      mitk::ProgressBar::GetInstance()->Progress(20);

    }
  }
  catch (const Exception& e)
  {
    MITK_ERROR << "Interpolation failed: " << e.what();
    interpolationResult = nullptr;
  }

  SetCPICacheSurface(interpolationResult, segmentationImage, labelValue, timeStep);
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::GetInterpolationResult(const MultiLabelSegmentation* segmentationImage, MultiLabelSegmentation::LabelValueType labelValue, TimeStepType timeStep)
{
  if (nullptr == segmentationImage)
  {
    mitkThrow() << "Cannot interpolate contours. No valid segmentation passed.";
  }

  std::shared_lock<std::shared_mutex> guard(cpiMutex);

  if (cpiMap.find(segmentationImage) == cpiMap.end())
  {
    mitkThrow() << "Cannot interpolate contours. Passed segmentation is not registered at controller.";
  }

  if (!segmentationImage->ExistLabel(labelValue))
  {
    mitkThrow() << "Cannot interpolate contours. None existent label request. Invalid label:" << labelValue;
  }

  if (!segmentationImage->GetTimeGeometry()->IsValidTimeStep(timeStep))
  {
    mitkThrow() << "Cannot interpolate contours. No valid time step requested. Invalid time step:" << timeStep;
  }

  const auto& currentImageContours = cpiMap.at(segmentationImage);

  auto finding = currentImageContours.find(labelValue);
  if (finding == currentImageContours.end())
  {
    return nullptr;
  }

  const auto& currentLabelContoursMap = finding->second;

  auto tsfinding = currentLabelContoursMap.find(timeStep);
  if (tsfinding == currentLabelContoursMap.end())
  {
    return nullptr;
  }

  return tsfinding->second.cachedSurface;
}

void mitk::SurfaceInterpolationController::SetDataStorage(DataStorage::Pointer ds)
{
  m_DataStorage = ds;
}

void mitk::SurfaceInterpolationController::SetDistanceImageVolume(unsigned int distImgVolume)
{
  m_DistanceImageVolume = distImgVolume;
}

mitk::MultiLabelSegmentation* mitk::SurfaceInterpolationController::GetCurrentSegmentation()
{
  return m_SelectedSegmentation.Lock();
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

void mitk::SurfaceInterpolationController::SetCurrentInterpolationSession(mitk::MultiLabelSegmentation* currentSegmentationImage)
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
      auto& controller = *this;
      const auto sender = selectedSegmentation.GetPointer();
      labelRemovedObserverTags[selectedSegmentation] = selectedSegmentation->AddObserver(mitk::LabelRemovedEvent(), [&controller, sender](const itk::EventObject& event)
        {
          controller.OnRemoveLabel(sender, event);
        });
    }
  }
}

void mitk::SurfaceInterpolationController::RemoveInterpolationSession(const mitk::MultiLabelSegmentation* segmentationImage)
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

void mitk::SurfaceInterpolationController::RemoveObserversInternal(const mitk::MultiLabelSegmentation* segmentationImage)
{
  auto pos = segmentationObserverTags.find(const_cast<mitk::MultiLabelSegmentation*>(segmentationImage));
  if (pos != segmentationObserverTags.end())
  {
    pos->first->RemoveObserver((*pos).second);
    segmentationObserverTags.erase(const_cast<mitk::MultiLabelSegmentation*>(segmentationImage));
  }

  auto pos2 = labelRemovedObserverTags.find(const_cast<mitk::MultiLabelSegmentation*>(segmentationImage));
  if (pos2 != labelRemovedObserverTags.end())
  {
    pos2->first->RemoveObserver((*pos2).second);
    labelRemovedObserverTags.erase(const_cast<mitk::MultiLabelSegmentation*>(segmentationImage));
  }
}

void mitk::SurfaceInterpolationController::RemoveAllInterpolationSessions()
{
  while (!cpiMap.empty())
  {
    this->RemoveInterpolationSession(cpiMap.begin()->first);
  }
}

void mitk::SurfaceInterpolationController::RemoveContours(const MultiLabelSegmentation* segmentationImage,
  mitk::Label::PixelType label,
  TimeStepType timeStep)
{
  if (nullptr == segmentationImage)
  {
    mitkThrow() << "Cannot remove contours. No valid segmentation passed.";
  }

  std::lock_guard<std::shared_mutex> guard(cpiMutex);

  auto segfinding = cpiMap.find(segmentationImage);

  if (segfinding != cpiMap.end())
  {
    auto& cpiLabelMap = cpiMap[segmentationImage];
    auto finding = cpiLabelMap.find(label);

    if (finding != cpiLabelMap.end())
    {
      cpiLabelMap[label].erase(timeStep);
    }

    if (m_DataStorage.IsNotNull())
    {
      //remove relevant plane nodes
      auto nodes = this->GetPlaneGeometryNodeFromDataStorage(GetSegmentationImageNodeInternal(this->m_DataStorage, segmentationImage), label, timeStep);

      this->m_DataStorage->Remove(nodes);
    }
    this->Modified();
  }
}

void mitk::SurfaceInterpolationController::RemoveContours(const MultiLabelSegmentation* segmentationImage,
  mitk::Label::PixelType label)
{
  if (nullptr == segmentationImage)
  {
    mitkThrow() << "Cannot remove contours. No valid segmentation passed.";
  }

  std::lock_guard<std::shared_mutex> guard(cpiMutex);

  auto finding = cpiMap.find(segmentationImage);
  if (finding != cpiMap.end())
  {
    cpiMap[segmentationImage].erase(label);

    if (m_DataStorage.IsNotNull())
    {
      //remove relevant plane nodes
      auto nodes = this->GetPlaneGeometryNodeFromDataStorage(GetSegmentationImageNodeInternal(this->m_DataStorage, segmentationImage), label);
      this->m_DataStorage->Remove(nodes);
    }
    this->Modified();
  }
}


void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller,
                                                                 const itk::EventObject & /*event*/)
{
  auto tempImage = dynamic_cast<mitk::MultiLabelSegmentation *>(const_cast<itk::Object *>(caller));
  if (tempImage)
  {
    this->RemoveInterpolationSession(tempImage);
  }
}

void mitk::SurfaceInterpolationController::OnRemoveLabel(const itk::Object* caller, const itk::EventObject& event)
{
  auto sendingSegmentation = dynamic_cast<const MultiLabelSegmentation*>(caller);

  auto removeEvent = dynamic_cast<const mitk::LabelRemovedEvent*>(&event);

  if (nullptr != sendingSegmentation && nullptr != removeEvent)
  {
    this->RemoveContours(sendingSegmentation, removeEvent->GetLabelValue());
  }
}

mitk::SurfaceInterpolationController::CPIVector* mitk::SurfaceInterpolationController::GetContours(MultiLabelSegmentation::LabelValueType labelValue, TimeStepType timeStep)
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
      return &(tsFinding->second.cpis);
    }
  }

  return nullptr;
}

std::vector<mitk::MultiLabelSegmentation::LabelValueType> mitk::SurfaceInterpolationController::GetAffectedLabels(const MultiLabelSegmentation* seg, TimeStepType timeStep, const PlaneGeometry* plane) const
{
  std::lock_guard<std::shared_mutex> guard(cpiMutex);

  std::vector<mitk::MultiLabelSegmentation::LabelValueType> result;

  auto finding = cpiMap.find(seg);
  if (finding == cpiMap.end()) return result;
  const auto& currentImageContours = cpiMap[seg];

  for (const auto& [label, contours] : currentImageContours)
  {
    auto tsFinding = contours.find(timeStep);
    if (tsFinding != contours.end())
    {
      const auto& cpis = contours.at(timeStep).cpis;
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
