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

#include "mitkContourUtils.h"

#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkInstantiateAccessFunctions.h"
#include "mitkToolManagerProvider.h"
#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include "ipSegmentation.h"



mitk::ContourUtils::ContourUtils()
{
}

mitk::ContourUtils::~ContourUtils()
{
}

void mitk::ContourUtils::ProjectContourTo2DSlice(const Geometry3D* sliceGeometry, ContourModel* contourIn3D, ContourModel* contourIn2D)
{
  if ( !sliceGeometry || !contourIn3D ) return;

  contourIn2D->Clear();
  contourIn2D->Initialize();

  for ( ContourModel::ConstVertexIterator iter = contourIn3D->Begin();
        iter != contourIn3D->End();
        ++iter )
  {
    Point3D pointIn2D;
    sliceGeometry->WorldToIndex( (*iter)->Coordinates, pointIn2D );
    contourIn2D->AddVertex( pointIn2D );
  }
}

void mitk::ContourUtils::BackProjectContourFrom2DSlice(const Geometry3D* sliceGeometry, ContourModel* contourIn2D, ContourModel* contourIn3D )
{
  if ( !sliceGeometry || !contourIn2D ) return;

  contourIn3D->Clear();
  contourIn3D->Initialize();

  ContourModel::ConstVertexIterator iter = contourIn2D->Begin();
  for ( ; iter != contourIn2D->End(); ++iter)
  {
    Point3D pointIn3D;
    sliceGeometry->IndexToWorld( (*iter)->Coordinates, pointIn3D );
    contourIn3D->AddVertex(pointIn3D);
  }
}

void mitk::ContourUtils::FillContourInSlice( ContourModel* projectedContour, Image* slice, int paintingPixelValue )
{
  // 1. Use ipSegmentation to draw a filled(!) contour into a new 8 bit 2D image, which will later be copied back to the slice.
  //    We don't work on the "real" working data, because ipSegmentation would restrict us to 8 bit images

  // convert the projected contour into a ipSegmentation format
  mitkIpInt4_t* picContour = new mitkIpInt4_t[2 * projectedContour->GetNumberOfVertices()];
//  const Contour::PathType::VertexListType* pointsIn2D = projectedContour->GetContourPath()->GetVertexList();
  unsigned int index(0);
  for ( ContourModel::ConstVertexIterator iter = projectedContour->Begin();
        iter != projectedContour->End();
        ++iter, ++index )
  {
    picContour[ 2 * index + 0 ] = static_cast<mitkIpInt4_t>( (*iter)->Coordinates[0] + 1.0 ); // +0.5 wahrscheinlich richtiger
    picContour[ 2 * index + 1 ] = static_cast<mitkIpInt4_t>( (*iter)->Coordinates[1] + 1.0 );
    //MITK_INFO << "mitk 2d [" << (*iter)[0] << ", " << (*iter)[1] << "]  pic [" << picContour[ 2*index+0] << ", " << picContour[ 2*index+1] << "]";
  }

  assert( slice->GetSliceData() );
  mitkIpPicDescriptor* originalPicSlice = mitkIpPicNew();
  CastToIpPicDescriptor( slice, originalPicSlice);
  mitkIpPicDescriptor* picSlice = ipMITKSegmentationNew( originalPicSlice );
  ipMITKSegmentationClear( picSlice );

  assert( originalPicSlice && picSlice );

  // here comes the actual contour filling algorithm (from ipSegmentation/Graphics Gems)
  ipMITKSegmentationCombineRegion ( picSlice, picContour, projectedContour->GetNumberOfVertices(), NULL, IPSEGMENTATION_OR,  1); // set to 1

  delete[] picContour;

  // 2. Copy the filled contour to the working data slice
  //    copy all pixels that are non-zero to the original image (not caring for the actual type of the working image). perhaps make the replace value a parameter ( -> general painting tool ).
  //    make the pic slice an mitk/itk image (as little ipPic code as possible), call a templated method with accessbyitk, iterate over the original and the modified slice

  Image::Pointer ipsegmentationModifiedSlice = Image::New();
  ipsegmentationModifiedSlice->Initialize( CastToImageDescriptor( picSlice ) );
  ipsegmentationModifiedSlice->SetSlice( picSlice->data );

  AccessFixedDimensionByItk_2( slice, ItkCopyFilledContourToSlice, 2, ipsegmentationModifiedSlice, paintingPixelValue );

  ipsegmentationModifiedSlice = NULL; // free MITK header information
  ipMITKSegmentationFree( picSlice ); // free actual memory
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ContourUtils::ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const mitk::Image* filledContourSlice, int overwritevalue )
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

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(toolManager);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(toolManager->GetWorkingData(0)->GetData());
  assert(workingImage);

  int activePixelValue = workingImage->GetActiveLabel()->GetValue();

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
      if ( inputIterator.Get() != 0 )
      {
        int targetValue = static_cast<int>(outputIterator.Get());
        if (!workingImage->GetLabel(targetValue)->GetLocked())
          outputIterator.Set(overwritevalue);
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
