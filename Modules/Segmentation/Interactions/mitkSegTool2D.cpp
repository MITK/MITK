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
#include "mitkPlanarCircle.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkLabelSetImage.h"
#include "mitkImageToContourFilter.h"
#include "mitkSurfaceBasedInterpolationController.h"
#include "mitkSliceBasedInterpolationController.h"
#include "mitkImageToContourModelSetFilter.h"

#include "usGetModuleContext.h"

//includes for resling and overwriting
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include <mitkDiffSliceOperationApplier.h>
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))
/*
#define InstantiateAccessFunction_ItkPasteSegmentation(pixelType, dim) \
  template void mitk::SegTool2D::ItkPasteSegmentation(itk::Image<pixelType,dim>*, const mitk::Image*, int pixelvalue);

//explicitly instantiate the 2D version of this method
InstantiateAccessFunctionForFixedDimension(mitk::SegTool2D::ItkPasteSegmentation, 2);
*/

mitk::SegTool2D::SegTool2D(const char* type)
:Tool(type),
m_LastEventSender(NULL),
m_LastEventSlice(-1),
m_Contourmarkername ("Position"),
m_ShowMarkerNodes (false),
m_3DInterpolationEnabled(false),
m_2DInterpolationEnabled(false)
{
  m_ProgressCommand = mitk::ToolCommand::New();
}

mitk::SegTool2D::~SegTool2D()
{
}

float mitk::SegTool2D::CanHandleEvent( InteractionEvent const *stateEvent) const
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>( stateEvent );
  if (!positionEvent) return 0.0;

  if ( positionEvent->GetSender()->GetMapperID() != BaseRenderer::Standard2D )
    return 0.0; // we don't want anything but 2D

  return 1.0;

//   //This are the mouse event that are used by the statemachine patterns for zooming and panning. This must be possible although a tool is activ
//   if (stateEvent->GetId() == EIDRIGHTMOUSEBTN || stateEvent->GetId() == EIDMIDDLEMOUSEBTN || stateEvent->GetId() == EIDRIGHTMOUSEBTNANDCTRL ||
//     stateEvent->GetId() == EIDMIDDLEMOUSERELEASE || stateEvent->GetId() == EIDRIGHTMOUSERELEASE || stateEvent->GetId() == EIDRIGHTMOUSEBTNANDMOUSEMOVE ||
//     stateEvent->GetId() == EIDMIDDLEMOUSEBTNANDMOUSEMOVE || stateEvent->GetId() == EIDCTRLANDRIGHTMOUSEBTNANDMOUSEMOVE || stateEvent->GetId() == EIDCTRLANDRIGHTMOUSEBTNRELEASE )
//   {
//     //Since the usual segmentation tools currently do not need right click interaction but the mitkDisplayVectorInteractor
//     return 0.0;
//   }
//   else
//   {
//     return 1.0;
//   }
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

mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const InteractionPositionEvent* positionEvent, const Image* image)
{
  if (!positionEvent) return NULL;

  assert( positionEvent->GetSender() ); // sure, right?
  unsigned int timeStep = positionEvent->GetSender()->GetTimeStep( image ); // get the timestep of the visible part (time-wise) of the image

  // first, we determine, which slice is affected
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );

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

  //specify the undo operation with the non edited slice
  m_undoOperation = new DiffSliceOperation(const_cast<mitk::Image*>(image), extractor->GetVtkOutput(), slice->GetGeometry(), timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));

  return slice;
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedWorkingSlice(const InteractionPositionEvent* positionEvent)
{
  Image* workingImage = dynamic_cast<Image*>(m_WorkingNode->GetData());
  assert(workingImage);

  return GetAffectedImageSliceAs2DImage( positionEvent, workingImage );
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const InteractionPositionEvent* positionEvent)
{
  DataNode* referenceNode( m_ToolManager->GetReferenceData(0) );
  assert(referenceNode);

  Image* referenceImage = dynamic_cast<Image*>(referenceNode->GetData());
  assert(referenceImage);

  return GetAffectedImageSliceAs2DImage( positionEvent, referenceImage );
}

void mitk::SegTool2D::WriteBackSegmentationResult (const InteractionPositionEvent* positionEvent, Image* slice)
{
  if ((!positionEvent) || (!slice)) return;

  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if( !planeGeometry ) return;

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  unsigned int timeStep = positionEvent->GetSender()->GetTimeStep( workingImage );

  if (m_2DInterpolationEnabled)
  {
    int clickedSliceDimension(-1);
    int clickedSliceIndex(-1);
    mitk::SegTool2D::DetermineAffectedImageSlice( workingImage, planeGeometry, clickedSliceDimension, clickedSliceIndex );
    mitk::SliceBasedInterpolationController* interpolator = mitk::SliceBasedInterpolationController::InterpolatorForImage(workingImage);
    if(interpolator)
    {
      try
      {
        interpolator->SetChangedSlice( slice, clickedSliceDimension, clickedSliceIndex, timeStep );
      }
      catch ( itk::ExceptionObject& e )
      {
        mitkThrow() << e.GetDescription();
        return;
      }
    }
  }

  if (m_3DInterpolationEnabled)
  {
    unsigned int pos = this->AddContourmarker(positionEvent);
    us::ServiceReference<PlanePositionManagerService> serviceRef = us::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
    PlanePositionManagerService* service = us::GetModuleContext()->GetService(serviceRef);
    mitk::SurfaceBasedInterpolationController* interpolator = mitk::SurfaceBasedInterpolationController::GetInstance();
    assert(interpolator);
    mitk::ImageToContourModelSetFilter::Pointer converter = mitk::ImageToContourModelSetFilter::New();
    converter->SetInput(slice);
    converter->SetContourValue(workingImage->GetActiveLabelIndex());
    converter->SetSliceGeometry(slice->GetGeometry());
    converter->Update();
    mitk::ContourModel::Pointer newContour = converter->GetOutput()->GetContourModelAt(0);
    newContour->DisconnectPipeline();
    if (newContour.IsNotNull() && newContour->GetNumberOfVertices())
    {
      try
      {
        interpolator->AddNewContour( newContour, service->GetPlanePosition(pos) );
      }
      catch ( itk::ExceptionObject& e )
      {
        mitkThrow() << e.GetDescription();
        return;
      }
    }
  }

  this->WriteBackSegmentationResult(planeGeometry, slice, timeStep);
  slice->DisconnectPipeline();
}

void mitk::SegTool2D::WriteBackSegmentationResult (const PlaneGeometry* planeGeometry, Image* slice, unsigned int timeStep)
{
  if(!planeGeometry || !slice) return;

  Image* workingImage = dynamic_cast<Image*>(m_WorkingNode->GetData());
  assert(workingImage);

  //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

  //Set the slice as 'input'
  reslice->SetInputSlice(slice->GetVtkImageData());

  //set overwrite mode to true to write back to the working image
  reslice->SetOverwriteMode(true);
  reslice->Modified();

  mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput( workingImage );
  extractor->SetTimeStep( timeStep );
  extractor->SetWorldGeometry( planeGeometry );
  extractor->SetVtkOutputRequest(true);
  extractor->SetResliceTransformByGeometry( workingImage->GetTimeGeometry()->GetGeometryForTimeStep( timeStep ) );
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch ( itk::ExceptionObject& e )
  {
    mitkThrow() << e.GetDescription();
    return;
  }

  //the working image was modified within the pipeline, but not marked so
  workingImage->Modified();
//  image->GetVtkImageData()->Modified();

  //specify the undo operation with the edited slice
  m_doOperation = new DiffSliceOperation(workingImage, extractor->GetVtkOutput(), slice->GetGeometry(), timeStep, const_cast<mitk::PlaneGeometry*>(planeGeometry));

  //create an operation event for the undo stack
  OperationEvent* undoStackItem = new OperationEvent( DiffSliceOperationApplier::GetInstance(), m_doOperation, m_undoOperation, "Segmentation" );

  //add it to the undo controller
  UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );

  //clear the pointers as the operation are stored in the undocontroller and also deleted from there
  m_undoOperation = NULL;
  m_doOperation = NULL;
}

void mitk::SegTool2D::SetShowMarkerNodes(bool status)
{
  m_ShowMarkerNodes = status;
}

void mitk::SegTool2D::SetEnable3DInterpolation(bool enabled)
{
  m_3DInterpolationEnabled = enabled;
}

void mitk::SegTool2D::SetEnable2DInterpolation(bool enabled)
{
  m_2DInterpolationEnabled = enabled;
}

unsigned int mitk::SegTool2D::AddContourmarker ( const InteractionPositionEvent* positionEvent )
{
  const mitk::Geometry2D* plane = dynamic_cast<const Geometry2D*> (dynamic_cast< const mitk::SlicedGeometry3D*>(
    positionEvent->GetSender()->GetSliceNavigationController()->GetCurrentGeometry3D())->GetGeometry2D(0));
  us::ServiceReference<PlanePositionManagerService> serviceRef =
          us::GetModuleContext()->GetServiceReference<PlanePositionManagerService>();
      PlanePositionManagerService* service = us::GetModuleContext()->GetService(serviceRef);
  unsigned int size = service->GetNumberOfPlanePositions();
  unsigned int id = service->AddNewPlanePosition(plane, positionEvent->GetSender()->GetSliceNavigationController()->GetSlice()->GetPos());

  mitk::PlanarCircle::Pointer contourMarker = mitk::PlanarCircle::New();
  mitk::Point2D p1;
  plane->Map(plane->GetCenter(), p1);
  mitk::Point2D p2 = p1;
  p2[0] -= plane->GetSpacing()[0];
  p2[1] -= plane->GetSpacing()[1];
  contourMarker->PlaceFigure( p1 );
  contourMarker->SetCurrentControlPoint( p1 );
  contourMarker->SetGeometry2D( const_cast<Geometry2D*>(plane));

  std::stringstream markerStream;

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
  rotatedContourNode->SetProperty( "planarfigure.drawcontrolpoints", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "planarfigure.drawname", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "planarfigure.drawoutline", BoolProperty::New(false));
  rotatedContourNode->SetProperty( "planarfigure.drawshadow", BoolProperty::New(false));

  if (plane)
  {
    if ( id ==  size )
    {
      m_ToolManager->GetDataStorage()->Add(rotatedContourNode, m_WorkingNode);
    }
    else
    {
      mitk::NodePredicateProperty::Pointer isMarker = mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true));

      mitk::DataStorage::SetOfObjects::ConstPointer markers = m_ToolManager->GetDataStorage()->GetDerivations(m_WorkingNode,isMarker);

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
      m_ToolManager->GetDataStorage()->Add(rotatedContourNode, m_WorkingNode);
    }
  }
  return id;
}

void mitk::SegTool2D::WritePreviewOnWorkingImage( Image* targetSlice, Image* sourceSlice, int paintingPixelValue, int timestep )
{
  if ((!targetSlice) || (!sourceSlice)) return;
  AccessFixedDimensionByItk_2( targetSlice, InternalWritePreviewOnWorkingImage, 2, sourceSlice, paintingPixelValue );
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::SegTool2D::InternalWritePreviewOnWorkingImage( itk::Image<TPixel,VImageDimension>* targetSlice, const mitk::Image* sourceSlice, int overwritevalue )
{
  typedef itk::Image<TPixel,VImageDimension> SliceType;

  typename SliceType::Pointer sourceSliceITK;
  CastToItkImage( sourceSlice, sourceSliceITK );

  // now the original slice and the ipSegmentation-painted slice are in the same format, and we can just copy all pixels that are non-zero
  typedef itk::ImageRegionIterator< SliceType >        OutputIteratorType;
  typedef itk::ImageRegionConstIterator< SliceType >   InputIteratorType;

  InputIteratorType inputIterator( sourceSliceITK, sourceSliceITK->GetLargestPossibleRegion() );
  OutputIteratorType outputIterator( targetSlice, targetSlice->GetLargestPossibleRegion() );

  outputIterator.GoToBegin();
  inputIterator.GoToBegin();

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
  assert (workingImage);

  int activePixelValue = workingImage->GetActiveLabelIndex();

  if (activePixelValue == 0) // if exterior is the active label
  {
    while ( !outputIterator.IsAtEnd() )
    {
      if (inputIterator.Get() != 0)
      {
        outputIterator.Set( overwritevalue );
      }
      ++outputIterator;
      ++inputIterator;
    }
  }
  else if (overwritevalue != 0) // if we are not erasing
  {
    while ( !outputIterator.IsAtEnd() )
    {
      int targetValue = static_cast<int>(outputIterator.Get());
      if ( inputIterator.Get() != 0 )
      {
        if (!workingImage->GetLabelLocked(targetValue))
          outputIterator.Set( overwritevalue );
      }

      ++outputIterator;
      ++inputIterator;
    }
  }
  else // if we are erasing
  {
    while ( !outputIterator.IsAtEnd() )
    {
      const int targetValue = outputIterator.Get();
      if (inputIterator.Get() != 0)
      {
        if (targetValue == activePixelValue)
          outputIterator.Set( overwritevalue );
      }

      ++outputIterator;
      ++inputIterator;
    }
  }
}