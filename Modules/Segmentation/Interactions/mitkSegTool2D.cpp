/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegTool2D.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkDataStorage.h"

#include "mitkPlaneGeometry.h"

// Include of the new ImageExtractor
#include "mitkMorphologicalOperations.h"
#include "mitkPlanarCircle.h"

#include "usGetModuleContext.h"

// Includes for 3DSurfaceInterpolation
#include "mitkImageTimeSelector.h"
#include "mitkImageToContourFilter.h"
#include "mitkSurfaceInterpolationController.h"

// includes for resling and overwriting
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include <mitkDiffSliceOperationApplier.h>

#include "mitkAbstractTransformGeometry.h"
#include "mitkLabelSetImage.h"

#include "mitkContourModelUtils.h"

#include "itkImageRegionIterator.h"

#define ROUND(a) ((a) > 0 ? (int)((a) + 0.5) : -(int)(0.5 - (a)))

bool mitk::SegTool2D::m_SurfaceInterpolationEnabled = true;

mitk::SegTool2D::SliceInformation::SliceInformation(const mitk::Image* aSlice, const mitk::PlaneGeometry* aPlane, mitk::TimeStepType aTimestep) :
  slice(aSlice), plane(aPlane), timestep(aTimestep)
{
}

mitk::SegTool2D::SegTool2D(const char *type, const us::Module *interactorModule)
  : Tool(type, interactorModule), m_Contourmarkername("Position")
{
  Tool::m_EventConfig = "DisplayConfigMITKNoCrosshair.xml";
}

mitk::SegTool2D::~SegTool2D()
{
}

bool mitk::SegTool2D::FilterEvents(InteractionEvent *interactionEvent, DataNode *)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);

  bool isValidEvent =
    (positionEvent && // Only events of type mitk::InteractionPositionEvent
     interactionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard2D // Only events from the 2D renderwindows
     );
  return isValidEvent;
}

bool mitk::SegTool2D::DetermineAffectedImageSlice(const Image *image,
                                                  const PlaneGeometry *plane,
                                                  int &affectedDimension,
                                                  int &affectedSlice)
{
  assert(image);
  assert(plane);

  // compare normal of plane to the three axis vectors of the image
  Vector3D normal = plane->GetNormal();
  Vector3D imageNormal0 = image->GetSlicedGeometry()->GetAxisVector(0);
  Vector3D imageNormal1 = image->GetSlicedGeometry()->GetAxisVector(1);
  Vector3D imageNormal2 = image->GetSlicedGeometry()->GetAxisVector(2);

  normal.Normalize();
  imageNormal0.Normalize();
  imageNormal1.Normalize();
  imageNormal2.Normalize();

  imageNormal0.SetVnlVector(vnl_cross_3d<ScalarType>(normal.GetVnlVector(), imageNormal0.GetVnlVector()));
  imageNormal1.SetVnlVector(vnl_cross_3d<ScalarType>(normal.GetVnlVector(), imageNormal1.GetVnlVector()));
  imageNormal2.SetVnlVector(vnl_cross_3d<ScalarType>(normal.GetVnlVector(), imageNormal2.GetVnlVector()));

  double eps(0.00001);
  // axial
  if (imageNormal2.GetNorm() <= eps)
  {
    affectedDimension = 2;
  }
  // sagittal
  else if (imageNormal1.GetNorm() <= eps)
  {
    affectedDimension = 1;
  }
  // frontal
  else if (imageNormal0.GetNorm() <= eps)
  {
    affectedDimension = 0;
  }
  else
  {
    affectedDimension = -1; // no idea
    return false;
  }

  // determine slice number in image
  BaseGeometry *imageGeometry = image->GetGeometry(0);
  Point3D testPoint = imageGeometry->GetCenter();
  Point3D projectedPoint;
  plane->Project(testPoint, projectedPoint);

  Point3D indexPoint;

  imageGeometry->WorldToIndex(projectedPoint, indexPoint);
  affectedSlice = ROUND(indexPoint[affectedDimension]);
  MITK_DEBUG << "indexPoint " << indexPoint << " affectedDimension " << affectedDimension << " affectedSlice "
             << affectedSlice;

  // check if this index is still within the image
  if (affectedSlice < 0 || affectedSlice >= static_cast<int>(image->GetDimension(affectedDimension)))
    return false;

  return true;
}

void mitk::SegTool2D::UpdateSurfaceInterpolation(const Image *slice,
                                                 const Image *workingImage,
                                                 const PlaneGeometry *plane,
                                                 bool detectIntersection)
{
  std::vector<SliceInformation> slices = { SliceInformation(slice, plane, 0)};
  Self::UpdateSurfaceInterpolation(slices, workingImage, detectIntersection);
}

void  mitk::SegTool2D::RemoveContourFromInterpolator(const SliceInformation& sliceInfo)
{
  mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo;
  contourInfo.contourNormal = sliceInfo.plane->GetNormal();
  contourInfo.contourPoint = sliceInfo.plane->GetOrigin();
  mitk::SurfaceInterpolationController::GetInstance()->RemoveContour(contourInfo);
}

void mitk::SegTool2D::UpdateSurfaceInterpolation(const std::vector<SliceInformation>& sliceInfos,
  const Image* workingImage,
  bool detectIntersection)
{
  if (!m_SurfaceInterpolationEnabled)
    return;

  //Remark: the ImageTimeSelector is just needed to extract a timestep/channel of
  //the image in order to get the image dimension (time dimension and channel dimension
  //stripped away). Therfore it is OK to always use time step 0 and channel 0
  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(workingImage);
  timeSelector->SetTimeNr(0);
  timeSelector->SetChannelNr(0);
  timeSelector->Update();
  const auto dimRefImg = timeSelector->GetOutput()->GetDimension();

  if (dimRefImg != 3)
    return;

  std::vector<mitk::Surface::Pointer> contourList;
  contourList.reserve(sliceInfos.size());

  ImageToContourFilter::Pointer contourExtractor = ImageToContourFilter::New();

  std::vector<SliceInformation> relevantSlices = sliceInfos;

  if (detectIntersection)
  {
    relevantSlices.clear();

    for (const auto& sliceInfo : sliceInfos)
    {
      // Test whether there is something to extract or whether the slice just contains intersections of others
      mitk::Image::Pointer slice2 = sliceInfo.slice->Clone();
      mitk::MorphologicalOperations::Erode(slice2, 2, mitk::MorphologicalOperations::Ball);

      contourExtractor->SetInput(slice2);
      contourExtractor->Update();
      mitk::Surface::Pointer contour = contourExtractor->GetOutput();

      if (contour->GetVtkPolyData()->GetNumberOfPoints() == 0)
      {
        Self::RemoveContourFromInterpolator(sliceInfo);
      }
      else
      {
        relevantSlices.push_back(sliceInfo);
      }
    }
  }

  if (relevantSlices.empty())
    return;

  for (const auto& sliceInfo : relevantSlices)
  {

    contourExtractor->SetInput(sliceInfo.slice);
    contourExtractor->Update();
    mitk::Surface::Pointer contour = contourExtractor->GetOutput();

    if (contour->GetVtkPolyData()->GetNumberOfPoints() == 0)
    {
      Self::RemoveContourFromInterpolator(sliceInfo);
    }
    else
    {
      contour->DisconnectPipeline();
      contourList.push_back(contour);
    }
  }
  mitk::SurfaceInterpolationController::GetInstance()->AddNewContours(contourList);
}



mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const InteractionPositionEvent *positionEvent, const Image *image, unsigned int component /*= 0*/)
{
  if (!positionEvent)
  {
    return nullptr;
  }

  assert(positionEvent->GetSender()); // sure, right?
  const auto timeStep = positionEvent->GetSender()->GetTimeStep(image); // get the timestep of the visible part (time-wise) of the image

  return GetAffectedImageSliceAs2DImage(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry(), image, timeStep, component);
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(const PlaneGeometry* planeGeometry, const Image* image, TimePointType timePoint, unsigned int component /*= 0*/)
{
  if (!image || !planeGeometry)
  {
    return nullptr;
  }

  if (!image->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return nullptr;

  return SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, image, image->GetTimeGeometry()->TimePointToTimeStep(timePoint), component);
}


mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const PlaneGeometry *planeGeometry, const Image *image, TimeStepType timeStep, unsigned int component /*= 0*/)
{
  if (!image || !planeGeometry)
  {
    return nullptr;
  }

  // Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  // set to false to extract a slice
  reslice->SetOverwriteMode(false);
  reslice->Modified();

  // use ExtractSliceFilter with our specific vtkImageReslice for overwriting and extracting
  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput(image);
  extractor->SetTimeStep(timeStep);
  extractor->SetWorldGeometry(planeGeometry);
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry(image->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));
  // additionally extract the given component
  // default is 0; the extractor checks for multi-component images
  extractor->SetComponent(component);

  extractor->Modified();
  extractor->Update();

  Image::Pointer slice = extractor->GetOutput();

  return slice;
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedWorkingSlice(const InteractionPositionEvent *positionEvent) const
{
  const auto workingNode = this->GetWorkingDataNode();
  if (!workingNode)
  {
    return nullptr;
  }

  const auto *workingImage = dynamic_cast<Image *>(workingNode->GetData());
  if (!workingImage)
  {
    return nullptr;
  }

  return GetAffectedImageSliceAs2DImage(positionEvent, workingImage);
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const InteractionPositionEvent *positionEvent) const
{
  DataNode* referenceNode = this->GetReferenceDataNode();
  if (!referenceNode)
  {
    return nullptr;
  }

  auto *referenceImage = dynamic_cast<Image *>(referenceNode->GetData());
  if (!referenceImage)
  {
    return nullptr;
  }

  int displayedComponent = 0;
  if (referenceNode->GetIntProperty("Image.Displayed Component", displayedComponent))
  {
    // found the displayed component
    return GetAffectedImageSliceAs2DImage(positionEvent, referenceImage, displayedComponent);
  }
  else
  {
    return GetAffectedImageSliceAs2DImage(positionEvent, referenceImage);
  }
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const PlaneGeometry* planeGeometry, TimeStepType timeStep) const
{
  DataNode* referenceNode = this->GetReferenceDataNode();
  if (!referenceNode)
  {
    return nullptr;
  }

  auto* referenceImage = dynamic_cast<Image*>(referenceNode->GetData());
  if (!referenceImage)
  {
    return nullptr;
  }

  int displayedComponent = 0;
  if (referenceNode->GetIntProperty("Image.Displayed Component", displayedComponent))
  {
    // found the displayed component
    return GetAffectedImageSliceAs2DImage(planeGeometry, referenceImage, timeStep, displayedComponent);
  }
  else
  {
    return GetAffectedImageSliceAs2DImage(planeGeometry, referenceImage, timeStep);
  }
}

void mitk::SegTool2D::Activated()
{
  Superclass::Activated();

  m_ToolManager->SelectedTimePointChanged +=
    mitk::MessageDelegate<mitk::SegTool2D>(this, &mitk::SegTool2D::OnTimePointChangedInternal);

  m_LastTimePointTriggered = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
}

void mitk::SegTool2D::Deactivated()
{
  m_ToolManager->SelectedTimePointChanged -=
    mitk::MessageDelegate<mitk::SegTool2D>(this, &mitk::SegTool2D::OnTimePointChangedInternal);
  Superclass::Deactivated();
}

void mitk::SegTool2D::OnTimePointChangedInternal()
{
  if (m_IsTimePointChangeAware && nullptr != this->GetWorkingDataNode())
  {
    const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

    if (timePoint != m_LastTimePointTriggered)
    {
      m_LastTimePointTriggered = timePoint;
      this->OnTimePointChanged();
    }
  }
}

void mitk::SegTool2D::OnTimePointChanged()
{
  //default implementation does nothing
}

mitk::DataNode* mitk::SegTool2D::GetWorkingDataNode() const
{
  if (nullptr != m_ToolManager)
  {
    return m_ToolManager->GetWorkingData(0);
  }
  return nullptr;
}

mitk::Image* mitk::SegTool2D::GetWorkingData() const
{
  auto node = this->GetWorkingDataNode();
  if (nullptr != node)
  {
    return dynamic_cast<Image*>(node->GetData());
  }
  return nullptr;
}

mitk::DataNode* mitk::SegTool2D::GetReferenceDataNode() const
{
  if (nullptr != m_ToolManager)
  {
    return m_ToolManager->GetReferenceData(0);
  }
  return nullptr;
}

mitk::Image* mitk::SegTool2D::GetReferenceData() const
{
  auto node = this->GetReferenceDataNode();
  if (nullptr != node)
  {
    return dynamic_cast<Image*>(node->GetData());
  }
  return nullptr;
}


void mitk::SegTool2D::WriteBackSegmentationResult(const InteractionPositionEvent *positionEvent, const Image * segmentationResult)
{
  if (!positionEvent)
    return;

  const PlaneGeometry *planeGeometry((positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  const auto *abstractTransformGeometry(
    dynamic_cast<const AbstractTransformGeometry *>(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));

  if (planeGeometry && segmentationResult && !abstractTransformGeometry)
  {
    const auto workingNode = this->GetWorkingDataNode();
    auto *image = dynamic_cast<Image *>(workingNode->GetData());
    const auto timeStep = positionEvent->GetSender()->GetTimeStep(image);
    this->WriteBackSegmentationResult(planeGeometry, segmentationResult, timeStep);
  }
}

void mitk::SegTool2D::WriteBackSegmentationResult(const DataNode* workingNode, const PlaneGeometry* planeGeometry, const Image* segmentationResult, TimeStepType timeStep)
{
  if (!planeGeometry || !segmentationResult)
    return;

  SliceInformation sliceInfo(segmentationResult, const_cast<mitk::PlaneGeometry*>(planeGeometry), timeStep);
  Self::WriteBackSegmentationResults(workingNode, { sliceInfo }, true);
}

void mitk::SegTool2D::WriteBackSegmentationResult(const PlaneGeometry *planeGeometry,
                                                  const Image * segmentationResult,
                                                  TimeStepType timeStep)
{
  if (!planeGeometry || !segmentationResult)
    return;

  SliceInformation sliceInfo(segmentationResult, const_cast<mitk::PlaneGeometry *>(planeGeometry), timeStep);
  WriteBackSegmentationResults({ sliceInfo }, true);
}

void mitk::SegTool2D::WriteBackSegmentationResults(const std::vector<SegTool2D::SliceInformation> &sliceList,
                                                  bool writeSliceToVolume)
{
  if (sliceList.empty())
  {
    return;
  }

  if (nullptr == m_LastEventSender)
  {
    MITK_WARN << "Cannot write tool results. Tool seems to be in an invalid state, as no interaction event was recieved but is expected.";
    return;
  }

  const auto workingNode = this->GetWorkingDataNode();

  mitk::SegTool2D::WriteBackSegmentationResults(workingNode, sliceList, writeSliceToVolume);

  // the first geometry is needed otherwise restoring the position is not working
  const auto* plane3 =
    dynamic_cast<const PlaneGeometry*>(dynamic_cast<const mitk::SlicedGeometry3D*>(
      m_LastEventSender->GetSliceNavigationController()->GetCurrentGeometry3D())
      ->GetPlaneGeometry(0));
  unsigned int slicePosition = m_LastEventSender->GetSliceNavigationController()->GetSlice()->GetPos();

  /* A cleaner solution would be to add a contour marker for each slice info. It currently
   does not work as the contour markers expect that the plane is always the plane of slice 0.
   Had not the time to do it properly no. Should be solved by T28146*/
  this->AddContourmarker(plane3, slicePosition);
}

void mitk::SegTool2D::WriteBackSegmentationResults(const DataNode* workingNode, const std::vector<SliceInformation>& sliceList, bool writeSliceToVolume)
{
  if (sliceList.empty())
  {
    return;
  }

  if (nullptr == workingNode)
  {
    mitkThrow() << "Cannot write slice to working node. Working node is invalid.";
  }

  auto* image = dynamic_cast<Image*>(workingNode->GetData());

  if (nullptr == image)
  {
    mitkThrow() << "Cannot write slice to working node. Working node does not contain an image.";
  }

  for (const auto& sliceInfo : sliceList)
  {
    if (writeSliceToVolume && nullptr != sliceInfo.plane && sliceInfo.slice.IsNotNull())
    {
      mitk::SegTool2D::WriteSliceToVolume(image, sliceInfo, true);
    }
  }

  mitk::SegTool2D::UpdateSurfaceInterpolation(sliceList, image, false);

  // also mark its node as modified (T27308). Can be removed if T27307
  // is properly solved
  if (workingNode != nullptr) workingNode->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool2D::WriteSliceToVolume(Image* workingImage, const PlaneGeometry* planeGeometry, const Image* slice, TimeStepType timeStep, bool allowUndo)
{
  SliceInformation sliceInfo(slice, planeGeometry, timeStep);
  WriteSliceToVolume(workingImage, sliceInfo , allowUndo);
}

void mitk::SegTool2D::WriteSliceToVolume(Image* workingImage, const SliceInformation &sliceInfo, bool allowUndo)
{
  if (nullptr == workingImage)
  {
    mitkThrow() << "Cannot write slice to working node. Working node does not contain an image.";
  }

  DiffSliceOperation* undoOperation = nullptr;

  if (allowUndo)
  {
    /*============= BEGIN undo/redo feature block ========================*/
    // Create undo operation by caching the not yet modified slices
    mitk::Image::Pointer originalSlice = GetAffectedImageSliceAs2DImage(sliceInfo.plane, workingImage, sliceInfo.timestep);
    undoOperation =
      new DiffSliceOperation(workingImage,
        originalSlice,
        dynamic_cast<SlicedGeometry3D*>(originalSlice->GetGeometry()),
        sliceInfo.timestep,
        sliceInfo.plane);
    /*============= END undo/redo feature block ========================*/
  }

  // Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk
  // reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

  // Set the slice as 'input'
  // casting const away is needed and OK as long the OverwriteMode of
  // mitkVTKImageOverwrite is true.
  // Reason: because then the input slice is not touched but
  // used to overwrite the input of the ExtractSliceFilter.
  auto noneConstSlice = const_cast<Image*>(sliceInfo.slice.GetPointer());
  reslice->SetInputSlice(noneConstSlice->GetVtkImageData());

  // set overwrite mode to true to write back to the image volume
  reslice->SetOverwriteMode(true);
  reslice->Modified();

  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput(workingImage);
  extractor->SetTimeStep(sliceInfo.timestep);
  extractor->SetWorldGeometry(sliceInfo.plane);
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry(workingImage->GetGeometry(sliceInfo.timestep));

  extractor->Modified();
  extractor->Update();

  // the image was modified within the pipeline, but not marked so
  workingImage->Modified();
  workingImage->GetVtkImageData()->Modified();

  if (allowUndo)
  {
    /*============= BEGIN undo/redo feature block ========================*/
    // specify the redo operation with the edited slice
    auto* doOperation =
      new DiffSliceOperation(workingImage,
        extractor->GetOutput(),
        dynamic_cast<SlicedGeometry3D*>(sliceInfo.slice->GetGeometry()),
        sliceInfo.timestep,
        sliceInfo.plane);

    // create an operation event for the undo stack
    OperationEvent* undoStackItem =
      new OperationEvent(DiffSliceOperationApplier::GetInstance(), doOperation, undoOperation, "Segmentation");

    // add it to the undo controller
    UndoStackItem::IncCurrObjectEventId();
    UndoStackItem::IncCurrGroupEventId();
    UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
    /*============= END undo/redo feature block ========================*/
  }

}


void mitk::SegTool2D::SetShowMarkerNodes(bool status)
{
  m_ShowMarkerNodes = status;
}

void mitk::SegTool2D::SetEnable3DInterpolation(bool enabled)
{
  m_SurfaceInterpolationEnabled = enabled;
}


int mitk::SegTool2D::AddContourmarker(const PlaneGeometry* planeGeometry, unsigned int sliceIndex)
{
  if (planeGeometry == nullptr)
    return -1;

  us::ServiceReference<PlanePositionManagerService> serviceRef =
    us::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
  PlanePositionManagerService *service = us::GetModuleContext()->GetService(serviceRef);

  unsigned int size = service->GetNumberOfPlanePositions();
  unsigned int id = service->AddNewPlanePosition(planeGeometry, sliceIndex);

  mitk::PlanarCircle::Pointer contourMarker = mitk::PlanarCircle::New();
  mitk::Point2D p1;
  planeGeometry->Map(planeGeometry->GetCenter(), p1);
  mitk::Point2D p2 = p1;
  p2[0] -= planeGeometry->GetSpacing()[0];
  p2[1] -= planeGeometry->GetSpacing()[1];
  contourMarker->PlaceFigure(p1);
  contourMarker->SetCurrentControlPoint(p1);
  contourMarker->SetPlaneGeometry(planeGeometry->Clone());

  std::stringstream markerStream;
  auto workingNode = this->GetWorkingDataNode();

  markerStream << m_Contourmarkername;
  markerStream << " ";
  markerStream << id + 1;

  DataNode::Pointer rotatedContourNode = DataNode::New();

  rotatedContourNode->SetData(contourMarker);
  rotatedContourNode->SetProperty("name", StringProperty::New(markerStream.str()));
  rotatedContourNode->SetProperty("isContourMarker", BoolProperty::New(true));
  rotatedContourNode->SetBoolProperty("PlanarFigureInitializedWindow", true, m_LastEventSender);
  rotatedContourNode->SetProperty("includeInBoundingBox", BoolProperty::New(false));
  rotatedContourNode->SetProperty("helper object", mitk::BoolProperty::New(!m_ShowMarkerNodes));
  rotatedContourNode->SetProperty("planarfigure.drawcontrolpoints", BoolProperty::New(false));
  rotatedContourNode->SetProperty("planarfigure.drawname", BoolProperty::New(false));
  rotatedContourNode->SetProperty("planarfigure.drawoutline", BoolProperty::New(false));
  rotatedContourNode->SetProperty("planarfigure.drawshadow", BoolProperty::New(false));

  if (planeGeometry)
  {
    if (id == size)
    {
      m_ToolManager->GetDataStorage()->Add(rotatedContourNode, workingNode);
    }
    else
    {
      mitk::NodePredicateProperty::Pointer isMarker =
        mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true));

      mitk::DataStorage::SetOfObjects::ConstPointer markers =
        m_ToolManager->GetDataStorage()->GetDerivations(workingNode, isMarker);

      for (auto iter = markers->begin(); iter != markers->end(); ++iter)
      {
        std::string nodeName = (*iter)->GetName();
        unsigned int t = nodeName.find_last_of(" ");
        unsigned int markerId = atof(nodeName.substr(t + 1).c_str()) - 1;
        if (id == markerId)
        {
          return id;
        }
      }
      m_ToolManager->GetDataStorage()->Add(rotatedContourNode, workingNode);
    }
  }
  return id;
}

void mitk::SegTool2D::InteractiveSegmentationBugMessage(const std::string &message) const
{
  MITK_ERROR << "********************************************************************************" << std::endl
             << " " << message << std::endl
             << "********************************************************************************" << std::endl
             << "  " << std::endl
             << " If your image is rotated or the 2D views don't really contain the patient image, try to press the "
                "button next to the image selection. "
             << std::endl
             << "  " << std::endl
             << " Please file a BUG REPORT: " << std::endl
             << " https://phabricator.mitk.org/" << std::endl
             << " Contain the following information:" << std::endl
             << "  - What image were you working on?" << std::endl
             << "  - Which region of the image?" << std::endl
             << "  - Which tool did you use?" << std::endl
             << "  - What did you do?" << std::endl
             << "  - What happened (not)? What did you expect?" << std::endl;
}

template <typename TPixel, unsigned int VImageDimension>
void InternalWritePreviewOnWorkingImage(itk::Image<TPixel, VImageDimension> *targetSlice,
                                        const mitk::Image *sourceSlice,
                                        mitk::Image *originalImage,
                                        int overwritevalue)
{
  typedef itk::Image<TPixel, VImageDimension> SliceType;

  typename SliceType::Pointer sourceSliceITK;
  CastToItkImage(sourceSlice, sourceSliceITK);

  // now the original slice and the ipSegmentation-painted slice are in the same format, and we can just copy all pixels
  // that are non-zero
  typedef itk::ImageRegionIterator<SliceType> OutputIteratorType;
  typedef itk::ImageRegionConstIterator<SliceType> InputIteratorType;

  InputIteratorType inputIterator(sourceSliceITK, sourceSliceITK->GetLargestPossibleRegion());
  OutputIteratorType outputIterator(targetSlice, targetSlice->GetLargestPossibleRegion());

  outputIterator.GoToBegin();
  inputIterator.GoToBegin();

  auto *workingImage = dynamic_cast<mitk::LabelSetImage *>(originalImage);
  assert(workingImage);

  int activePixelValue = workingImage->GetActiveLabel()->GetValue();

  if (activePixelValue == 0) // if exterior is the active label
  {
    while (!outputIterator.IsAtEnd())
    {
      if (inputIterator.Get() != 0)
      {
        outputIterator.Set(overwritevalue);
      }
      ++outputIterator;
      ++inputIterator;
    }
  }
  else if (overwritevalue != 0) // if we are not erasing
  {
    while (!outputIterator.IsAtEnd())
    {
      auto targetValue = static_cast<int>(outputIterator.Get());
      if (inputIterator.Get() != 0)
      {
        if (!workingImage->GetLabel(targetValue)->GetLocked())
        {
          outputIterator.Set(overwritevalue);
        }
      }
      if (targetValue == overwritevalue)
      {
        outputIterator.Set(inputIterator.Get());
      }

      ++outputIterator;
      ++inputIterator;
    }
  }
  else // if we are erasing
  {
    while (!outputIterator.IsAtEnd())
    {
      const int targetValue = outputIterator.Get();
      if (inputIterator.Get() != 0)
      {
        if (targetValue == activePixelValue)
          outputIterator.Set(overwritevalue);
      }

      ++outputIterator;
      ++inputIterator;
    }
  }
}

void mitk::SegTool2D::WritePreviewOnWorkingImage(
  Image *targetSlice, const Image *sourceSlice, const Image *workingImage, int paintingPixelValue)
{
  if (nullptr == targetSlice)
  {
    mitkThrow() << "Cannot write preview on working image. Target slice does not point to a valid instance.";
  }

  if (nullptr == sourceSlice)
  {
    mitkThrow() << "Cannot write preview on working image. Source slice does not point to a valid instance.";
  }

  if (nullptr == workingImage)
  {
    mitkThrow() << "Cannot write preview on working image. Working image does not point to a valid instance.";
  }

  auto constVtkSource = sourceSlice->GetVtkImageData();
  /*Need to const cast because Vtk interface does not support const correctly.
   (or I am not experienced enough to use it correctly)*/
  auto nonConstVtkSource = const_cast<vtkImageData*>(constVtkSource);

  ContourModelUtils::FillSliceInSlice(nonConstVtkSource, targetSlice->GetVtkImageData(), workingImage, paintingPixelValue);
}
