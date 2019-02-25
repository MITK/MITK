/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkSegTool2D.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkDataStorage.h"

#include "mitkPlaneGeometry.h"

#include "mitkExtractDirectedPlaneImageFilter.h"
#include "mitkExtractImageFilter.h"

// Include of the new ImageExtractor
#include "mitkExtractDirectedPlaneImageFilterNew.h"
#include "mitkMorphologicalOperations.h"
#include "mitkOverwriteDirectedPlaneImageFilter.h"
#include "mitkOverwriteSliceImageFilter.h"
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
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include "mitkLabelSetImage.h"

#define ROUND(a) ((a) > 0 ? (int)((a) + 0.5) : -(int)(0.5 - (a)))

bool mitk::SegTool2D::m_SurfaceInterpolationEnabled = true;

mitk::SegTool2D::SegTool2D(const char *type, const us::Module *interactorModule)
  : Tool(type, interactorModule), m_LastEventSender(nullptr), m_LastEventSlice(0), m_Contourmarkername("Position"), m_ShowMarkerNodes(false)
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
  if (!m_SurfaceInterpolationEnabled)
    return;

  ImageToContourFilter::Pointer contourExtractor = ImageToContourFilter::New();
  mitk::Surface::Pointer contour;

  if (detectIntersection)
  {
    // Test whether there is something to extract or whether the slice just contains intersections of others
    mitk::Image::Pointer slice2 = slice->Clone();
    mitk::MorphologicalOperations::Erode(slice2, 2, mitk::MorphologicalOperations::Ball);

    contourExtractor->SetInput(slice2);
    contourExtractor->Update();
    contour = contourExtractor->GetOutput();

    if (contour->GetVtkPolyData()->GetNumberOfPoints() == 0)
    {
      // Remove contour!
      mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo;
      contourInfo.contourNormal = plane->GetNormal();
      contourInfo.contourPoint = plane->GetOrigin();
      mitk::SurfaceInterpolationController::GetInstance()->RemoveContour(contourInfo);
      return;
    }
  }

  contourExtractor->SetInput(slice);
  contourExtractor->Update();
  contour = contourExtractor->GetOutput();

  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(workingImage);
  timeSelector->SetTimeNr(0);
  timeSelector->SetChannelNr(0);
  timeSelector->Update();
  Image::Pointer dimRefImg = timeSelector->GetOutput();

  if (contour->GetVtkPolyData()->GetNumberOfPoints() != 0 && dimRefImg->GetDimension() == 3)
  {
    mitk::SurfaceInterpolationController::GetInstance()->AddNewContour(contour);
    contour->DisconnectPipeline();
  }
  else
  {
    // Remove contour!
    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo;
    contourInfo.contourNormal = plane->GetNormal();
    contourInfo.contourPoint = plane->GetOrigin();
    mitk::SurfaceInterpolationController::GetInstance()->RemoveContour(contourInfo);
  }
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const InteractionPositionEvent *positionEvent, const Image *image, unsigned int component /*= 0*/)
{
  if (!positionEvent)
  {
    return nullptr;
  }

  assert(positionEvent->GetSender()); // sure, right?
  unsigned int timeStep = positionEvent->GetSender()->GetTimeStep(image); // get the timestep of the visible part (time-wise) of the image

  return GetAffectedImageSliceAs2DImage(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry(), image, timeStep, component);
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const PlaneGeometry *planeGeometry, const Image *image, unsigned int timeStep, unsigned int component /*= 0*/)
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

mitk::Image::Pointer mitk::SegTool2D::GetAffectedWorkingSlice(const InteractionPositionEvent *positionEvent)
{
  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  if (!workingNode)
  {
    return nullptr;
  }

  auto *workingImage = dynamic_cast<Image *>(workingNode->GetData());
  if (!workingImage)
  {
    return nullptr;
  }

  return GetAffectedImageSliceAs2DImage(positionEvent, workingImage);
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const InteractionPositionEvent *positionEvent)
{
  DataNode *referenceNode(m_ToolManager->GetReferenceData(0));
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

void mitk::SegTool2D::WriteBackSegmentationResult(const InteractionPositionEvent *positionEvent, Image *slice)
{
  if (!positionEvent)
    return;

  const PlaneGeometry *planeGeometry((positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  const auto *abstractTransformGeometry(
    dynamic_cast<const AbstractTransformGeometry *>(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));

  if (planeGeometry && slice && !abstractTransformGeometry)
  {
    DataNode *workingNode(m_ToolManager->GetWorkingData(0));
    auto *image = dynamic_cast<Image *>(workingNode->GetData());
    unsigned int timeStep = positionEvent->GetSender()->GetTimeStep(image);
    this->WriteBackSegmentationResult(planeGeometry, slice, timeStep);
  }
}

void mitk::SegTool2D::WriteBackSegmentationResult(const PlaneGeometry *planeGeometry,
                                                  Image *slice,
                                                  unsigned int timeStep)
{
  if (!planeGeometry || !slice)
    return;

  SliceInformation sliceInfo(slice, const_cast<mitk::PlaneGeometry *>(planeGeometry), timeStep);
  this->WriteSliceToVolume(sliceInfo);
  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  auto *image = dynamic_cast<Image *>(workingNode->GetData());

  this->UpdateSurfaceInterpolation(slice, image, planeGeometry, false);

  if (m_SurfaceInterpolationEnabled)
    this->AddContourmarker();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool2D::WriteBackSegmentationResult(const std::vector<SegTool2D::SliceInformation> &sliceList,
                                                  bool writeSliceToVolume)
{
  std::vector<mitk::Surface::Pointer> contourList;
  contourList.reserve(sliceList.size());
  ImageToContourFilter::Pointer contourExtractor = ImageToContourFilter::New();

  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  auto *image = dynamic_cast<Image *>(workingNode->GetData());

  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(image);
  timeSelector->SetTimeNr(0);
  timeSelector->SetChannelNr(0);
  timeSelector->Update();
  Image::Pointer dimRefImg = timeSelector->GetOutput();

  for (unsigned int i = 0; i < sliceList.size(); ++i)
  {
    SliceInformation currentSliceInfo = sliceList.at(i);
    if (writeSliceToVolume)
      this->WriteSliceToVolume(currentSliceInfo);
    if (m_SurfaceInterpolationEnabled && dimRefImg->GetDimension() == 3)
    {
      currentSliceInfo.slice->DisconnectPipeline();
      contourExtractor->SetInput(currentSliceInfo.slice);
      contourExtractor->Update();
      mitk::Surface::Pointer contour = contourExtractor->GetOutput();
      contour->DisconnectPipeline();

      contourList.push_back(contour);
    }
  }
  mitk::SurfaceInterpolationController::GetInstance()->AddNewContours(contourList);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool2D::WriteSliceToVolume(const mitk::SegTool2D::SliceInformation &sliceInfo)
{
  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  auto *image = dynamic_cast<Image *>(workingNode->GetData());

  /*============= BEGIN undo/redo feature block ========================*/
  // Create undo operation by caching the not yet modified slices
  mitk::Image::Pointer originalSlice = GetAffectedImageSliceAs2DImage(sliceInfo.plane, image, sliceInfo.timestep);
  auto *undoOperation =
    new DiffSliceOperation(image,
                           originalSlice,
                           dynamic_cast<SlicedGeometry3D *>(originalSlice->GetGeometry()),
                           sliceInfo.timestep,
                           sliceInfo.plane);
  /*============= END undo/redo feature block ========================*/

  // Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk
  // reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

  // Set the slice as 'input'
  reslice->SetInputSlice(sliceInfo.slice->GetVtkImageData());

  // set overwrite mode to true to write back to the image volume
  reslice->SetOverwriteMode(true);
  reslice->Modified();

  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput(image);
  extractor->SetTimeStep(sliceInfo.timestep);
  extractor->SetWorldGeometry(sliceInfo.plane);
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry(image->GetGeometry(sliceInfo.timestep));

  extractor->Modified();
  extractor->Update();

  // the image was modified within the pipeline, but not marked so
  image->Modified();
  image->GetVtkImageData()->Modified();

  /*============= BEGIN undo/redo feature block ========================*/
  // specify the undo operation with the edited slice
  auto *doOperation =
    new DiffSliceOperation(image,
                           extractor->GetOutput(),
                           dynamic_cast<SlicedGeometry3D *>(sliceInfo.slice->GetGeometry()),
                           sliceInfo.timestep,
                           sliceInfo.plane);

  // create an operation event for the undo stack
  OperationEvent *undoStackItem =
    new OperationEvent(DiffSliceOperationApplier::GetInstance(), doOperation, undoOperation, "Segmentation");

  // add it to the undo controller
  UndoStackItem::IncCurrObjectEventId();
  UndoStackItem::IncCurrGroupEventId();
  UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);

  // clear the pointers as the operation are stored in the undocontroller and also deleted from there
  undoOperation = nullptr;
  doOperation = nullptr;
  /*============= END undo/redo feature block ========================*/
}

void mitk::SegTool2D::SetShowMarkerNodes(bool status)
{
  m_ShowMarkerNodes = status;
}

void mitk::SegTool2D::SetEnable3DInterpolation(bool enabled)
{
  m_SurfaceInterpolationEnabled = enabled;
}

int mitk::SegTool2D::AddContourmarker()
{
  if (m_LastEventSender == nullptr)
    return -1;

  us::ServiceReference<PlanePositionManagerService> serviceRef =
    us::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
  PlanePositionManagerService *service = us::GetModuleContext()->GetService(serviceRef);

  unsigned int slicePosition = m_LastEventSender->GetSliceNavigationController()->GetSlice()->GetPos();

  // the first geometry is needed otherwise restoring the position is not working
  const auto *plane =
    dynamic_cast<const PlaneGeometry *>(dynamic_cast<const mitk::SlicedGeometry3D *>(
                                          m_LastEventSender->GetSliceNavigationController()->GetCurrentGeometry3D())
                                          ->GetPlaneGeometry(0));

  unsigned int size = service->GetNumberOfPlanePositions();
  unsigned int id = service->AddNewPlanePosition(plane, slicePosition);

  mitk::PlanarCircle::Pointer contourMarker = mitk::PlanarCircle::New();
  mitk::Point2D p1;
  plane->Map(plane->GetCenter(), p1);
  mitk::Point2D p2 = p1;
  p2[0] -= plane->GetSpacing()[0];
  p2[1] -= plane->GetSpacing()[1];
  contourMarker->PlaceFigure(p1);
  contourMarker->SetCurrentControlPoint(p1);
  contourMarker->SetPlaneGeometry(const_cast<PlaneGeometry *>(plane));

  std::stringstream markerStream;
  mitk::DataNode *workingNode(m_ToolManager->GetWorkingData(0));

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

  if (plane)
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

void mitk::SegTool2D::InteractiveSegmentationBugMessage(const std::string &message)
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
  Image *targetSlice, Image *sourceSlice, mitk::Image *workingImage, int paintingPixelValue, int)
{
  if ((!targetSlice) || (!sourceSlice))
    return;
  AccessFixedDimensionByItk_3(
    targetSlice, InternalWritePreviewOnWorkingImage, 2, sourceSlice, workingImage, paintingPixelValue);
}
