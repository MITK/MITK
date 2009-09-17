/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkContourUtils.h"
#include "mitkImageCast.h"
#include "ipSegmentation.h"

// explicitly instantiate the 2D version of this method
InstantiateAccessFunctionForFixedDimension_2( mitk::ContourUtils::ItkCopyFilledContourToSlice, 2, const mitk::Image*, int );
    
mitk::ContourUtils::ContourUtils()
{
}

mitk::ContourUtils::~ContourUtils()
{
}

mitk::Contour::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
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
      LOG_INFO << "**" << currentPointIn3D << " is " << projectedPointIn2D << " --> correct it (TODO)" << std::endl;
    }

    projectedContour->AddVertex( projectedPointIn2D );
  }

  return projectedContour;
}

mitk::Contour::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(Image* slice, Contour* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
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



void mitk::ContourUtils::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  // 1. Use ipSegmentation to draw a filled(!) contour into a new 8 bit 2D image, which will later be copied back to the slice.
  //    We don't work on the "real" working data, because ipSegmentation would restrict us to 8 bit images
  
  // convert the projected contour into a ipSegmentation format
  mitkIpInt4_t* picContour = new mitkIpInt4_t[2 * projectedContour->GetNumberOfPoints()];
  const Contour::PathType::VertexListType* pointsIn2D = projectedContour->GetContourPath()->GetVertexList();
  unsigned int index(0);
  for ( Contour::PathType::VertexListType::const_iterator iter = pointsIn2D->begin(); 
        iter != pointsIn2D->end();
        ++iter, ++index )
  {
    picContour[ 2 * index + 0 ] = static_cast<mitkIpInt4_t>( (*iter)[0] + 0.5 );
    picContour[ 2 * index + 1 ] = static_cast<mitkIpInt4_t>( (*iter)[1] + 0.5 );
  }

  assert( sliceImage->GetSliceData() );
  mitkIpPicDescriptor* originalPicSlice = sliceImage->GetSliceData()->GetPicDescriptor();
  mitkIpPicDescriptor* picSlice = ipMITKSegmentationNew( originalPicSlice );
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

template<typename TPixel, unsigned int VImageDimension>
void mitk::ContourUtils::ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const Image* filledContourSlice, int overwritevalue )
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

