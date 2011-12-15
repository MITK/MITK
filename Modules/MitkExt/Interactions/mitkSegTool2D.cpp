/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

#include "mitkGetModuleContext.h"

//Includes for 3DSurfaceInterpolation
#include "mitkImageToContourFilter.h"
#include "mitkSurfaceInterpolationController.h"


#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

mitk::SegTool2D::SegTool2D(const char* type)
:Tool(type),
 m_LastEventSender(NULL),
 m_LastEventSlice(0),
 m_Contourmarkername ("Position"),
 m_ShowMarkerNodes (true),
 m_3DInterpolationEnabled (true)
{
  // great magic numbers
  CONNECT_ACTION( 80, OnMousePressed );
  CONNECT_ACTION( 90, OnMouseMoved );
  CONNECT_ACTION( 42, OnMouseReleased );
  CONNECT_ACTION( 49014, OnInvertLogic );
}

mitk::SegTool2D::~SegTool2D()
{
}

bool mitk::SegTool2D::OnMousePressed (Action*, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( positionEvent->GetSender()->GetMapperID() != BaseRenderer::Standard2D ) return false; // we don't want anything but 2D

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  return true;
}

bool mitk::SegTool2D::OnMouseMoved   (Action*, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( m_LastEventSender != positionEvent->GetSender() ) return false;
  if ( m_LastEventSlice  != m_LastEventSender->GetSlice() ) return false;

  return true;
}

bool mitk::SegTool2D::OnMouseReleased(Action*, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( m_LastEventSender != positionEvent->GetSender() ) return false;
  if ( m_LastEventSlice  != m_LastEventSender->GetSlice() ) return false;

  return true;
}

bool mitk::SegTool2D::OnInvertLogic(Action*, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( m_LastEventSender != positionEvent->GetSender() ) return false;
  if ( m_LastEventSlice  != m_LastEventSender->GetSlice() ) return false;

  return true;
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

  imageNormal0.Set_vnl_vector( vnl_cross_3d<ScalarType>(normal.Get_vnl_vector(),imageNormal0.Get_vnl_vector()) );
  imageNormal1.Set_vnl_vector( vnl_cross_3d<ScalarType>(normal.Get_vnl_vector(),imageNormal1.Get_vnl_vector()) );
  imageNormal2.Set_vnl_vector( vnl_cross_3d<ScalarType>(normal.Get_vnl_vector(),imageNormal2.Get_vnl_vector()) );

  double eps( 0.00001 );
  // transversal
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
  Geometry3D* imageGeometry = image->GetGeometry(0);
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

mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const PositionEvent* positionEvent, const Image* image)
{
  if (!positionEvent) return NULL;

  assert( positionEvent->GetSender() ); // sure, right?
  unsigned int timeStep = positionEvent->GetSender()->GetTimeStep( image ); // get the timestep of the visible part (time-wise) of the image

  // first, we determine, which slice is affected
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );

  if ( !image || !planeGeometry ) return NULL;

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );
  if ( DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) )
  {
    try
    {
      // now we extract the correct slice from the volume, resulting in a 2D image
      ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
      extractor->SetInput( image );
      extractor->SetSliceDimension( affectedDimension );
      extractor->SetSliceIndex( affectedSlice );
      extractor->SetTimeStep( timeStep );
      extractor->Update();

      // here we have a single slice that can be modified
      Image::Pointer slice = extractor->GetOutput();

      //Workaround because of bug #7079
      Point3D origin = slice->GetGeometry()->GetOrigin();

      int affectedDimension(-1);

      if(positionEvent->GetSender()->GetRenderWindow() == mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))
      {
          affectedDimension = 2;
      }
      if(positionEvent->GetSender()->GetRenderWindow() == mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2"))
      {
          affectedDimension = 0;
      }
      if(positionEvent->GetSender()->GetRenderWindow() == mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))
      {
          affectedDimension = 1;
      }

      if (affectedDimension != -1)
      {
          origin[affectedDimension] = planeGeometry->GetOrigin()[affectedDimension];
          slice->GetGeometry()->SetOrigin(origin);
      }
      //Workaround end

      return slice;
    }
    catch(...)
    {
      // not working
      return NULL;
    }
  }
  else
  {
    ExtractDirectedPlaneImageFilterNew::Pointer newExtractor = ExtractDirectedPlaneImageFilterNew::New();
    newExtractor->SetInput( image );
    newExtractor->SetActualInputTimestep( timeStep );
    newExtractor->SetCurrentWorldGeometry2D( planeGeometry );
    newExtractor->Update();
    Image::Pointer slice = newExtractor->GetOutput();
    return slice;
  }
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedWorkingSlice(const PositionEvent* positionEvent)
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if ( !workingNode ) return NULL;

  Image* workingImage = dynamic_cast<Image*>(workingNode->GetData());
  if ( !workingImage ) return NULL;

  return GetAffectedImageSliceAs2DImage( positionEvent, workingImage );
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const PositionEvent* positionEvent)
{
  DataNode* referenceNode( m_ToolManager->GetReferenceData(0) );
  if ( !referenceNode ) return NULL;

  Image* referenceImage = dynamic_cast<Image*>(referenceNode->GetData());
  if ( !referenceImage ) return NULL;

  return GetAffectedImageSliceAs2DImage( positionEvent, referenceImage );
}

void mitk::SegTool2D::WriteBackSegmentationResult (const PositionEvent* positionEvent, Image* slice)
{
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  Image* image = dynamic_cast<Image*>(workingNode->GetData());

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice );

  if (affectedDimension != -1) {
    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( slice );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();
  }
  else {
    OverwriteDirectedPlaneImageFilter::Pointer slicewriter = OverwriteDirectedPlaneImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( false );
    slicewriter->SetSliceImage( slice );
    slicewriter->SetPlaneGeometry3D( slice->GetGeometry() );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();
  }
  if ( m_3DInterpolationEnabled )
  {
    slice->DisconnectPipeline();
    unsigned int pos = this->AddContourmarker(positionEvent);
    ImageToContourFilter::Pointer contourExtractor = ImageToContourFilter::New();
    contourExtractor->SetInput(slice);
    contourExtractor->Update();
    mitk::Surface::Pointer contour = contourExtractor->GetOutput();
    mitk::ServiceReference serviceRef = mitk::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
    PlanePositionManagerService* service = dynamic_cast<PlanePositionManagerService*>(mitk::GetModuleContext()->GetService(serviceRef));
    mitk::SurfaceInterpolationController::GetInstance()->AddNewContour( contour, service->GetPlanePosition(pos));
    contour->DisconnectPipeline();
  }
}

void mitk::SegTool2D::SetShowMarkerNodes(bool status)
{
    m_ShowMarkerNodes = status;
}

void mitk::SegTool2D::Enable3DInterpolation(bool status)
{
    m_3DInterpolationEnabled = status;
}

unsigned int mitk::SegTool2D::AddContourmarker ( const PositionEvent* positionEvent )
{
  const mitk::Geometry2D* plane = dynamic_cast<const Geometry2D*> (dynamic_cast< const mitk::SlicedGeometry3D*>(
    positionEvent->GetSender()->GetSliceNavigationController()->GetCurrentGeometry3D())->GetGeometry2D(0));

  mitk::ServiceReference serviceRef = mitk::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
  PlanePositionManagerService* service = dynamic_cast<PlanePositionManagerService*>(mitk::GetModuleContext()->GetService(serviceRef));
  unsigned int size = service->GetNumberOfPlanePositions();
  unsigned int id = service->AddNewPlanePosition(plane, positionEvent->GetSender()->GetSliceNavigationController()->GetSlice()->GetPos());

  mitk::PlanarCircle::Pointer contourMarker = mitk::PlanarCircle::New();
  contourMarker->SetGeometry2D( const_cast<Geometry2D*>(plane));

  std::stringstream markerStream;
  mitk::DataNode* workingNode (m_ToolManager->GetWorkingData(0));

  markerStream << m_Contourmarkername ;
  markerStream << " ";
  markerStream << id+1;

  DataNode::Pointer rotatedContourNode = DataNode::New();

  rotatedContourNode->SetData(contourMarker);
  rotatedContourNode->SetProperty( "name", StringProperty::New(markerStream.str()) );
  rotatedContourNode->SetProperty( "isContourMarker", BoolProperty::New(true));
  rotatedContourNode->SetBoolProperty( "PlanarFigureInitializedWindow", true, positionEvent->GetSender() );
  rotatedContourNode->SetProperty( "includeInBoundingBox", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "helper object", mitk::BoolProperty::New(!m_ShowMarkerNodes));

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

