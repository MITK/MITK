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

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"

#include "mitkDataStorage.h"
#include "mitkBaseRenderer.h"

#include "mitkPlaneGeometry.h"
#include "mitkRenderingManager.h"

#include "mitkExtractImageFilter.h"
#include "ipSegmentation.h"

#include <itkImageRegionIterator.h>

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

// explicitly instantiate the 2D version of this method
InstantiateAccessFunctionForFixedDimension_2( mitk::SegTool2D::ItkCopyFilledContourToSlice, 2, const mitk::Image*, int );

mitk::SegTool2D::SegTool2D(const char* type)
:SegTool3D(type),
 m_FeedbackContourVisible(false),
 m_LastEventSender(NULL),
 m_LastEventSlice(0)
{
  // great magic numbers
  CONNECT_ACTION( 80, OnMousePressed );
  CONNECT_ACTION( 90, OnMouseMoved );
  CONNECT_ACTION( 42, OnMouseReleased );
  CONNECT_ACTION( 49014, OnInvertLogic );
    
  m_FeedbackContour = Contour::New();
  m_FeedbackContourNode = DataTreeNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetProperty("name", new StringProperty("One of SegTool2D's feedback nodes"));
  m_FeedbackContourNode->SetProperty("visible", new BoolProperty(true));
  m_FeedbackContourNode->SetProperty("layer", new IntProperty(1000));
  m_FeedbackContourNode->SetProperty("Width", new FloatProperty(1)); // uppercase! Slim line looks very accurate :-)

  // set explicitly visible=false for all 3D renderer (that exist already ...)
  const RenderingManager::RenderWindowVector& renderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for (RenderingManager::RenderWindowVector::const_iterator iter = renderWindows.begin();
       iter != renderWindows.end();
       ++iter)
  {
    if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == BaseRenderer::Standard3D )
    //if ( (*iter)->GetRenderer()->GetMapperID() == BaseRenderer::Standard3D )
    {
      m_FeedbackContourNode->SetProperty("visible", new BoolProperty(false), mitk::BaseRenderer::GetInstance((*iter)));
    }
  }

  SetFeedbackContourColorDefault();
}

mitk::SegTool2D::~SegTool2D()
{
}

void mitk::SegTool2D::SetFeedbackContourColor( float r, float g, float b )
{
  m_FeedbackContourNode->SetProperty("color", new ColorProperty(r, g, b));
}

void mitk::SegTool2D::SetFeedbackContourColorDefault()
{
  m_FeedbackContourNode->SetProperty("color", new ColorProperty(0.0/255.0, 255.0/255.0, 0.0/255.0));
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

mitk::Contour* mitk::SegTool2D::GetFeedbackContour()
{
  return m_FeedbackContour;
}

void mitk::SegTool2D::SetFeedbackContour(Contour& contour)
{
  m_FeedbackContour = &contour;
  m_FeedbackContourNode->SetData( m_FeedbackContour );
}

void mitk::SegTool2D::SetFeedbackContourVisible(bool visible)
{
  if ( m_FeedbackContourVisible == visible ) return; // nothing changed

  if (visible)
  {
    DataStorage::GetInstance()->Add( m_FeedbackContourNode );
  }
  else
  {
    DataStorage::GetInstance()->Remove( m_FeedbackContourNode );
  }

  m_FeedbackContourVisible = visible;
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

  // check if this index is still within the image
  if ( affectedSlice < 0 || affectedSlice > static_cast<int>(image->GetDimension(affectedDimension)) ) return false;
 
  return true;
}
    
mitk::Image::Pointer mitk::SegTool2D::GetAffectedImageSliceAs2DImage(const PositionEvent* positionEvent, const Image* image)
{
  if (!positionEvent) return NULL;
  
  assert( positionEvent->GetSender() ); // sure, right?

  // first, we determine, which slice is affected
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );

  if ( !image || !planeGeometry ) return NULL;

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  if ( DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) )
  {
    // now we extract the correct slice from the volume, resulting in a 2D image
    ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
    extractor->SetInput( image );
    extractor->SetSliceDimension( affectedDimension );
    extractor->SetSliceIndex( affectedSlice );
    extractor->Update();

    // here we have a single slice that can be modified
    Image::Pointer slice = extractor->GetOutput();
    
    return slice;
  }
  else
  {
    return NULL;
  }
}

mitk::Contour::Pointer mitk::SegTool2D::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  if ( !slice || !contourIn3D ) return NULL;

  Contour::Pointer projectedContour = Contour::New();

  const Contour::PathType::VertexListType* pointsIn3D = contourIn3D->GetContourPath()->GetVertexList();
  const Geometry3D* sliceGeometry = slice->GetGeometry();
  for ( Contour::PathType::VertexListType::const_iterator iter = pointsIn3D->begin(); 
        iter != pointsIn3D->end();
        ++iter )
  {
    Contour::PathType::VertexType currentPointIn3DITK = *iter;
    Point3D currentPointIn3D;
    for (int i = 0; i < 3; ++i) currentPointIn3D[i] = currentPointIn3DITK[i];

    Point3D projectedPointIn2D;
    projectedPointIn2D.Fill(0);
    sliceGeometry->WorldToIndex( currentPointIn3D, projectedPointIn2D );

    if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) && constrainToInside )
    {
      std::cout << "**" << currentPointIn3D << " is " << projectedPointIn2D << " --> correct it (TODO)" << std::endl;
    }

    projectedContour->AddVertex( projectedPointIn2D );
  }

  return projectedContour;
}
    
mitk::Contour::Pointer mitk::SegTool2D::BackProjectContourFrom2DSlice(Image* slice, Contour* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  if ( !slice || !contourIn2D ) return NULL;

  Contour::Pointer worldContour = Contour::New();

  const Contour::PathType::VertexListType* pointsIn2D = contourIn2D->GetContourPath()->GetVertexList();
  const Geometry3D* sliceGeometry = slice->GetGeometry();
  for ( Contour::PathType::VertexListType::const_iterator iter = pointsIn2D->begin(); 
        iter != pointsIn2D->end();
        ++iter )
  {
    Contour::PathType::VertexType currentPointIn3DITK = *iter;
    Point3D currentPointIn2D;
    for (int i = 0; i < 3; ++i) currentPointIn2D[i] = currentPointIn3DITK[i];
    
    Point3D worldPointIn3D;
    worldPointIn3D.Fill(0);
    sliceGeometry->IndexToWorld( currentPointIn2D, worldPointIn3D );

    worldContour->AddVertex( worldPointIn3D );
  }

  return worldContour;
}
    
mitk::Image::Pointer mitk::SegTool2D::GetAffectedWorkingSlice(const PositionEvent* positionEvent)
{
  DataTreeNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if ( !workingNode ) return NULL;
  
  Image* workingImage = dynamic_cast<Image*>(workingNode->GetData());
  if ( !workingImage ) return NULL;
  
  return GetAffectedImageSliceAs2DImage( positionEvent, workingImage );
}

mitk::Image::Pointer mitk::SegTool2D::GetAffectedReferenceSlice(const PositionEvent* positionEvent)
{
  DataTreeNode* referenceNode( m_ToolManager->GetReferenceData(0) );
  if ( !referenceNode ) return NULL;
  
  Image* referenceImage = dynamic_cast<Image*>(referenceNode->GetData());
  if ( !referenceImage ) return NULL;
  
  return GetAffectedImageSliceAs2DImage( positionEvent, referenceImage );
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::SegTool2D::ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const Image* filledContourSlice, int overwritevalue )
{
  typedef itk::Image<TPixel,VImageDimension> SliceType;

  typename SliceType::Pointer filledContourSliceITK;
  CastToItkImage( filledContourSlice, filledContourSliceITK );

  // now the original slice and the ipSegmentation-painted slice are in the same format, and we can just copy all pixels that are non-zero
  typedef itk::ImageRegionIterator< SliceType >       OutputIteratorType;
  typedef itk::ImageRegionConstIterator< SliceType >   InputIteratorType;

  InputIteratorType inputIterator( filledContourSliceITK, filledContourSliceITK->GetLargestPossibleRegion() );
  OutputIteratorType outputIterator( originalSlice, originalSlice->GetLargestPossibleRegion() );
  
  outputIterator.GoToBegin();
  inputIterator.GoToBegin();
  
  while ( !outputIterator.IsAtEnd() )
  {
    if ( inputIterator.Get() != 0 )
    {
      outputIterator.Set( overwritevalue );
    }
 
    ++outputIterator;
    ++inputIterator;
  }
} 

void mitk::SegTool2D::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
{
    // 1. Use ipSegmentation to draw a filled(!) contour into a new 8 bit 2D image, which will later be copied back to the slice.
    //    We don't work on the "real" working data, because ipSegmentation would restrict us to 8 bit images
    
    // convert the projected contour into a ipSegmentation format
    ipInt4_t* picContour = new ipInt4_t[2 * projectedContour->GetNumberOfPoints()];
    const Contour::PathType::VertexListType* pointsIn2D = projectedContour->GetContourPath()->GetVertexList();
    unsigned int index(0);
    for ( Contour::PathType::VertexListType::const_iterator iter = pointsIn2D->begin(); 
          iter != pointsIn2D->end();
          ++iter, ++index )
    {
      picContour[ 2 * index + 0 ] = static_cast<ipInt4_t>( (*iter)[0] + 0.5 );
      picContour[ 2 * index + 1 ] = static_cast<ipInt4_t>( (*iter)[1] + 0.5 );
    }
 
    assert( sliceImage->GetSliceData() );
    ipPicDescriptor* originalPicSlice = sliceImage->GetSliceData()->GetPicDescriptor();
    ipPicDescriptor* picSlice = ipMITKSegmentationNew( originalPicSlice );
    ipMITKSegmentationClear( picSlice );

    assert( originalPicSlice && picSlice );

    // here comes the actual contour filling algorithm (from ipSegmentation/Graphics Gems)
    ipMITKSegmentationCombineRegion ( picSlice, picContour, projectedContour->GetNumberOfPoints(), NULL, IPSEGMENTATION_OR,  1); // set to 1

    delete[] picContour;

    // 2. Copy the filled contour to the working data slice
    //    copy all pixels that are non-zero to the original image (not caring for the actual type of the working image). perhaps make the replace value a parameter ( -> general painting tool ).
    //    make the pic slice an mitk/itk image (as little ipPic code as possible), call a templated method with accessbyitk, iterate over the original and the modified slice

    Image::Pointer ipsegmentationModifiedSlice = Image::New();
    ipsegmentationModifiedSlice->Initialize( picSlice );
    ipsegmentationModifiedSlice->SetPicSlice( picSlice );

    AccessFixedDimensionByItk_2( sliceImage, ItkCopyFilledContourToSlice, 2, ipsegmentationModifiedSlice, paintingPixelValue );

    ipsegmentationModifiedSlice = NULL; // free MITK header information
    ipMITKSegmentationFree( picSlice ); // free actual memory

}

void mitk::SegTool2D::SliceBasedSegmentationBugMessage( const std::string& message )
{
  std::cout << "********************************************************************************" << std::endl;
  std::cout << " " << message << std::endl;
  std::cout << "********************************************************************************" << std::endl;
  std::cout << "  " << std::endl;
  std::cout << " If your image is rotated or the 2D views don't really contain the patient image, try to press the button next to the image selection. " << std::endl;
  std::cout << "  " << std::endl;
  std::cout << " Please file a BUG REPORT: " << std::endl;
  std::cout << " http://makalu.inet.dkfz-heidelberg.de/bugzilla/enter_bug.cgi?product=ReLiver&component=QmitkSliceBasedSegmentation&assigned_to=d.maleike%40dkfz-heidelberg.de" << std::endl;
  std::cout << " Contain the following information:" << std::endl;
  std::cout << "  - What image were you working on?" << std::endl;
  std::cout << "  - Which region of the image?" << std::endl;
  std::cout << "  - Which tool did you use?" << std::endl;
  std::cout << "  - What did you do?" << std::endl;
  std::cout << "  - What happened (not)? What did you expect?" << std::endl;
  std::cout << "  " << std::endl;
}

