#include "mitkFeedbackContourTool.h"

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"

#include "mitkDataStorage.h"
#include "mitkBaseRenderer.h"

#include "mitkGeometry3D.h"
#include "mitkRenderingManager.h"

#include "ipSegmentation.h"

#include <itkImageRegionIterator.h>

// explicitly instantiate the 2D version of this method
InstantiateAccessFunctionForFixedDimension_2( mitk::FeedbackContourTool::ItkCopyFilledContourToSlice, 2, const mitk::Image*, int );

mitk::FeedbackContourTool::FeedbackContourTool(const char* type)
:SegTool2D(type),
 m_FeedbackContourVisible(false)
{
  m_FeedbackContour = Contour::New();
  m_FeedbackContourNode = DataTreeNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetProperty("name", StringProperty::New("One of FeedbackContourTool's feedback nodes"));
  m_FeedbackContourNode->SetProperty("visible", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("layer", IntProperty::New(1000));
  m_FeedbackContourNode->SetProperty("Width", FloatProperty::New(1)); // uppercase! Slim line looks very accurate :-)

  // set explicitly visible=false for all 3D renderer (that exist already ...)
  const RenderingManager::RenderWindowVector& renderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for (RenderingManager::RenderWindowVector::const_iterator iter = renderWindows.begin();
       iter != renderWindows.end();
       ++iter)
  {
    if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == BaseRenderer::Standard3D )
    //if ( (*iter)->GetRenderer()->GetMapperID() == BaseRenderer::Standard3D )
    {
      m_FeedbackContourNode->SetProperty("visible", BoolProperty::New(false), mitk::BaseRenderer::GetInstance((*iter)));
    }
  }

  SetFeedbackContourColorDefault();
}

mitk::FeedbackContourTool::~FeedbackContourTool()
{
}

void mitk::FeedbackContourTool::SetFeedbackContourColor( float r, float g, float b )
{
  m_FeedbackContourNode->SetProperty("color", ColorProperty::New(r, g, b));
}

void mitk::FeedbackContourTool::SetFeedbackContourColorDefault()
{
  m_FeedbackContourNode->SetProperty("color", ColorProperty::New(0.0/255.0, 255.0/255.0, 0.0/255.0));
}

mitk::Contour* mitk::FeedbackContourTool::GetFeedbackContour()
{
  return m_FeedbackContour;
}

void mitk::FeedbackContourTool::SetFeedbackContour(Contour& contour)
{
  m_FeedbackContour = &contour;
  m_FeedbackContourNode->SetData( m_FeedbackContour );
}

void mitk::FeedbackContourTool::SetFeedbackContourVisible(bool visible)
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

mitk::Contour::Pointer mitk::FeedbackContourTool::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
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
    
mitk::Contour::Pointer mitk::FeedbackContourTool::BackProjectContourFrom2DSlice(Image* slice, Contour* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
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

template<typename TPixel, unsigned int VImageDimension>
void mitk::FeedbackContourTool::ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const Image* filledContourSlice, int overwritevalue )
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

void mitk::FeedbackContourTool::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
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

 
