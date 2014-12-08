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

#include "mitkDataStorage.h"
#include "mitkBaseRenderer.h"

#include "mitkPlaneGeometry.h"

#include "mitkExtractImageFilter.h"
#include "mitkExtractDirectedPlaneImageFilter.h"

//Include of the new ImageExtractor
#include "mitkExtractDirectedPlaneImageFilterNew.h"
#include "mitkPlanarCircle.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkOverwriteDirectedPlaneImageFilter.h"
#include "mitkMorphologicalOperations.h"

#include "usGetModuleContext.h"

//Includes for 3DSurfaceInterpolation
#include "mitkImageToContourFilter.h"
#include "mitkSurfaceInterpolationController.h"

//includes for resling and overwriting
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include <mitkDiffSliceOperationApplier.h>
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"

#include "mitkAbstractTransformGeometry.h"

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

bool mitk::SegTool2D::m_SurfaceInterpolationEnabled = true;

mitk::SegTool2D::SegTool2D(const char* type)
:Tool(type),
m_LastEventSender(NULL),
m_LastEventSlice(0),
m_Contourmarkername ("Position"),
m_ShowMarkerNodes (false)
{
}

mitk::SegTool2D::~SegTool2D()
{
}

bool mitk::SegTool2D::FilterEvents(InteractionEvent* interactionEvent, DataNode*dataNode)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>( interactionEvent );

  bool isValidEvent = (
                       positionEvent && // Only events of type mitk::InteractionPositionEvent
                       interactionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard2D // Only events from the 2D renderwindows
                      );
  return isValidEvent;
}



bool mitk::SegTool2D::DetermineAffectedImageSlice( const Image* image, const PlaneGeometry* plane, int& affectedDimension, int& affectedSlice )
{
  assert(image);
  assert(plane);

  // compare normal of plane to the three axis vectors of the image
  Vector3D normal       = plane->GetNormal();
  Vector3D imageNormal0 = image->GetSlicedGeometry()->GetAxisVector(0);
  Vector3D imageNormal1 = image->GetSlicedGeometry()->GetAxisVector(1);
  Vector3D imageNormal2 = image->GetSlicedGeometry()->GetAxisVector(2);

  normal.Normalize();
  imageNormal0.Normalize();
  imageNormal1.Normalize();
  imageNormal2.Normalize();

  imageNormal0.SetVnlVector( vnl_cross_3d<ScalarType>(normal.GetVnlVector(),imageNormal0.GetVnlVector()) );
  imageNormal1.SetVnlVector( vnl_cross_3d<ScalarType>(normal.GetVnlVector(),imageNormal1.GetVnlVector()) );
  imageNormal2.SetVnlVector( vnl_cross_3d<ScalarType>(normal.GetVnlVector(),imageNormal2.GetVnlVector()) );

  double eps( 0.00001 );
  // axial
  if ( imageNormal2.GetNorm() <= eps )
  {
    affectedDimension = 2;
  }
  // sagittal
  else if ( imageNormal1.GetNorm() <= eps )
  {
    affectedDimension = 1;
  }
  // frontal
  else if ( imageNormal0.GetNorm() <= eps )
  {
    affectedDimension = 0;
  }
  else
  {
    affectedDimension = -1; // no idea
    return false;
  }

  // determine slice number in image
  BaseGeometry* imageGeometry = image->GetGeometry(0);
  Point3D testPoint = imageGeometry->GetCenter();
  Point3D projectedPoint;
  plane->Project( testPoint, projectedPoint );

  Point3D indexPoint;

  imageGeometry->WorldToIndex( projectedPoint, indexPoint );
  affectedSlice = ROUND( indexPoint[affectedDimension] );
  MITK_DEBUG << "indexPoint " << indexPoint << " affectedDimension " << affectedDimension << " affectedSlice " << affectedSlice;

  // check if this index is still within the image
  if ( affectedSlice < 0 || affectedSlice >= static_cast<int>(image->GetDimension(affectedDimension)) ) return false;

  return true;
}

void mitk::SegTool2D::UpdateSurfaceInterpolation (const Image* slice, const Image* workingImage, const PlaneGeometry *plane, bool detectIntersection)
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

  if (contour->GetVtkPolyData()->GetNumberOfPoints() != 0 && workingImage->GetDimension() == 3)
  {
    mitk::SurfaceInterpolationController::GetInstance()->AddNewContour( contour );
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


mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const InteractionPositionEvent* positionEvent, const Image* image)
{
  if (!positionEvent) return NULL;

  assert( positionEvent->GetSender() ); // sure, right?
  unsigned int timeStep = positionEvent->GetSender()->GetTimeStep( image ); // get the timestep of the visible part (time-wise) of the image

  // first, we determine, which slice is affected
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldPlaneGeometry() ) );

  return this->GetAffectedImageSliceAs2DImage(planeGeometry, image, timeStep);
}


mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const PlaneGeometry* planeGeometry, const Image* image, unsigned int timeStep)
{
  if ( !image || !planeGeometry ) return NULL;

  //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  //set to false to extract a slice
  reslice->SetOverwriteMode(false);
  reslice->Modified();

  //use ExtractSliceFilter with our specific vtkImageReslice for overwriting and extracting
  mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput( image );
  extractor->SetTimeStep( timeStep );
  extractor->SetWorldGeometry( planeGeometry );
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry( image->GetTimeGeometry()->GetGeometryForTimeStep( timeStep ) );

  extractor->Modified();
  extractor->Update();

  Image::Pointer slice = extractor->GetOutput();

  /*============= BEGIN undo feature block ========================*/
  //specify the undo operation with the non edited slice
  m_undoOperation = new DiffSliceOperation(const_cast<mitk::Image*>(image), extractor->GetVtkOutput(), dynamic_cast<SlicedGeometry3D*>(slice->GetGeometry()), timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));
  /*============= END undo feature block ========================*/

  return slice;
}


mitk::Image::Pointer mitk::SegTool2D::GetAffectedWorkingSlice(const InteractionPositionEvent* positionEvent)
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if ( !workingNode ) return NULL;

  Image* workingImage = dynamic_cast<Image*>(workingNode->GetData());
  if ( !workingImage ) return NULL;

  return GetAffectedImageSliceAs2DImage( positionEvent, workingImage );
}


mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const InteractionPositionEvent* positionEvent)
{
  DataNode* referenceNode( m_ToolManager->GetReferenceData(0) );
  if ( !referenceNode ) return NULL;

  Image* referenceImage = dynamic_cast<Image*>(referenceNode->GetData());
  if ( !referenceImage ) return NULL;

  return GetAffectedImageSliceAs2DImage( positionEvent, referenceImage );
}

void mitk::SegTool2D::WriteBackSegmentationResult (const InteractionPositionEvent* positionEvent, Image* slice)
{
  if(!positionEvent) return;

  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldPlaneGeometry() ) );
  const AbstractTransformGeometry* abstractTransformGeometry( dynamic_cast<const AbstractTransformGeometry*> (positionEvent->GetSender()->GetCurrentWorldPlaneGeometry() ) );

  if( planeGeometry && slice && !abstractTransformGeometry)
  {
    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
    Image* image = dynamic_cast<Image*>(workingNode->GetData());
    unsigned int timeStep = positionEvent->GetSender()->GetTimeStep( image );
    this->WriteBackSegmentationResult(planeGeometry, slice, timeStep);
  }

}


void mitk::SegTool2D::WriteBackSegmentationResult (const PlaneGeometry* planeGeometry, Image* slice, unsigned int timeStep)
{
  if(!planeGeometry || !slice) return;

  SliceInformation sliceInfo (slice, const_cast<mitk::PlaneGeometry*>(planeGeometry), timeStep);
  this->WriteSliceToVolume(sliceInfo);
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  Image* image = dynamic_cast<Image*>(workingNode->GetData());

  this->UpdateSurfaceInterpolation(slice, image, planeGeometry, false);

  if (m_SurfaceInterpolationEnabled)
    this->AddContourmarker();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool2D::WriteBackSegmentationResult(std::vector<mitk::SegTool2D::SliceInformation> sliceList, bool writeSliceToVolume)
{
  std::vector<mitk::Surface::Pointer> contourList;
  contourList.reserve(sliceList.size());
  ImageToContourFilter::Pointer contourExtractor = ImageToContourFilter::New();

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  Image* image = dynamic_cast<Image*>(workingNode->GetData());


  for (unsigned int i = 0; i < sliceList.size(); ++i)
  {
    SliceInformation currentSliceInfo = sliceList.at(i);
    if(writeSliceToVolume)
      this->WriteSliceToVolume(currentSliceInfo);
    if (m_SurfaceInterpolationEnabled && image->GetDimension() == 3)
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

void mitk::SegTool2D::WriteSliceToVolume(mitk::SegTool2D::SliceInformation sliceInfo)
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  Image* image = dynamic_cast<Image*>(workingNode->GetData());

  //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

  //Set the slice as 'input'
  reslice->SetInputSlice(sliceInfo.slice->GetVtkImageData());

  //set overwrite mode to true to write back to the image volume
  reslice->SetOverwriteMode(true);
  reslice->Modified();

  mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput( image );
  extractor->SetTimeStep( sliceInfo.timestep );
  extractor->SetWorldGeometry( sliceInfo.plane );
  extractor->SetVtkOutputRequest(true);
  extractor->SetResliceTransformByGeometry( image->GetGeometry( sliceInfo.timestep ) );

  extractor->Modified();
  extractor->Update();

  //the image was modified within the pipeline, but not marked so
  image->Modified();
  image->GetVtkImageData()->Modified();

  /*============= BEGIN undo feature block ========================*/
  //specify the undo operation with the edited slice
  m_doOperation = new DiffSliceOperation(image, extractor->GetVtkOutput(),dynamic_cast<SlicedGeometry3D*>(sliceInfo.slice->GetGeometry()), sliceInfo.timestep, sliceInfo.plane);

  //create an operation event for the undo stack
  OperationEvent* undoStackItem = new OperationEvent( DiffSliceOperationApplier::GetInstance(), m_doOperation, m_undoOperation, "Segmentation" );

  //add it to the undo controller
  UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );

  //clear the pointers as the operation are stored in the undocontroller and also deleted from there
  m_undoOperation = NULL;
  m_doOperation = NULL;
  /*============= END undo feature block ========================*/
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
  if (m_LastEventSender == NULL)
    return -1;

  us::ServiceReference<PlanePositionManagerService> serviceRef =
      us::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
  PlanePositionManagerService* service = us::GetModuleContext()->GetService(serviceRef);

  unsigned int slicePosition = m_LastEventSender->GetSliceNavigationController()->GetSlice()->GetPos();

  // the first geometry is needed otherwise restoring the position is not working
  const mitk::PlaneGeometry* plane = dynamic_cast<const PlaneGeometry*> (dynamic_cast< const mitk::SlicedGeometry3D*>(
   m_LastEventSender->GetSliceNavigationController()->GetCurrentGeometry3D())->GetPlaneGeometry(0));

  unsigned int size = service->GetNumberOfPlanePositions();
  unsigned int id = service->AddNewPlanePosition(plane, slicePosition);

  mitk::PlanarCircle::Pointer contourMarker = mitk::PlanarCircle::New();
  mitk::Point2D p1;
  plane->Map(plane->GetCenter(), p1);
  mitk::Point2D p2 = p1;
  p2[0] -= plane->GetSpacing()[0];
  p2[1] -= plane->GetSpacing()[1];
  contourMarker->PlaceFigure( p1 );
  contourMarker->SetCurrentControlPoint( p1 );
  contourMarker->SetPlaneGeometry( const_cast<PlaneGeometry*>(plane));

  std::stringstream markerStream;
  mitk::DataNode* workingNode (m_ToolManager->GetWorkingData(0));

  markerStream << m_Contourmarkername ;
  markerStream << " ";
  markerStream << id+1;

  DataNode::Pointer rotatedContourNode = DataNode::New();

  rotatedContourNode->SetData(contourMarker);
  rotatedContourNode->SetProperty( "name", StringProperty::New(markerStream.str()) );
  rotatedContourNode->SetProperty( "isContourMarker", BoolProperty::New(true));
  rotatedContourNode->SetBoolProperty( "PlanarFigureInitializedWindow", true, m_LastEventSender );
  rotatedContourNode->SetProperty( "includeInBoundingBox", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "helper object", mitk::BoolProperty::New(!m_ShowMarkerNodes));
  rotatedContourNode->SetProperty( "planarfigure.drawcontrolpoints", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "planarfigure.drawname", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "planarfigure.drawoutline", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "planarfigure.drawshadow", BoolProperty::New(false));

  if (plane)
  {

    if ( id ==  size )
    {
      m_ToolManager->GetDataStorage()->Add(rotatedContourNode, workingNode);
    }
    else
    {
      mitk::NodePredicateProperty::Pointer isMarker = mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true));

      mitk::DataStorage::SetOfObjects::ConstPointer markers = m_ToolManager->GetDataStorage()->GetDerivations(workingNode,isMarker);

      for ( mitk::DataStorage::SetOfObjects::const_iterator iter = markers->begin();
        iter != markers->end();
        ++iter)
      {
        std::string nodeName = (*iter)->GetName();
        unsigned int t = nodeName.find_last_of(" ");
        unsigned int markerId = atof(nodeName.substr(t+1).c_str())-1;
        if(id == markerId)
        {
          return id;
        }
      }
      m_ToolManager->GetDataStorage()->Add(rotatedContourNode, workingNode);
    }
  }
  return id;
}

void mitk::SegTool2D::InteractiveSegmentationBugMessage( const std::string& message )
{
  MITK_ERROR << "********************************************************************************" << std::endl
    << " " << message << std::endl
    << "********************************************************************************" << std::endl
    << "  " << std::endl
    << " If your image is rotated or the 2D views don't really contain the patient image, try to press the button next to the image selection. " << std::endl
    << "  " << std::endl
    << " Please file a BUG REPORT: " << std::endl
    << " http://bugs.mitk.org" << std::endl
    << " Contain the following information:" << std::endl
    << "  - What image were you working on?" << std::endl
    << "  - Which region of the image?" << std::endl
    << "  - Which tool did you use?" << std::endl
    << "  - What did you do?" << std::endl
    << "  - What happened (not)? What did you expect?" << std::endl;
}
