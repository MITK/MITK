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
#include "mitkLegacyAdaptors.h"
#include "ipSegmentation.h"

#define InstantiateAccessFunction_ItkCopyFilledContourToSlice(pixelType, dim) \
  template void mitk::ContourUtils::ItkCopyFilledContourToSlice(itk::Image<pixelType,dim>*, const mitk::Image*, int);

// explicitly instantiate the 2D version of this method
InstantiateAccessFunctionForFixedDimension(ItkCopyFilledContourToSlice, 2);

mitk::ContourUtils::ContourUtils()
{
}

mitk::ContourUtils::~ContourUtils()
{
}


mitk::ContourModel::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  mitk::Contour::PointsContainerIterator it = contourIn3D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn3D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return this->ProjectContourTo2DSlice(slice, contour, false/*not used*/, constrainToInside );
}

mitk::ContourModel::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  if ( !slice || !contourIn3D ) return NULL;

  ContourModel::Pointer projectedContour = ContourModel::New();
  projectedContour->Initialize(*contourIn3D);

  const BaseGeometry* sliceGeometry = slice->GetGeometry();

  int numberOfTimesteps = contourIn3D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
    ContourModel::VertexIterator iter = contourIn3D->Begin(currentTimestep);
    ContourModel::VertexIterator end = contourIn3D->End(currentTimestep);

    while( iter != end)
    {
      Point3D currentPointIn3D = (*iter)->Coordinates;

      Point3D projectedPointIn2D;
      projectedPointIn2D.Fill(0.0);
      sliceGeometry->WorldToIndex( currentPointIn3D, projectedPointIn2D );
      // MITK_INFO << "world point " << currentPointIn3D << " in index is " << projectedPointIn2D;

      if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) && constrainToInside )
      {
        MITK_INFO << "**" << currentPointIn3D << " is " << projectedPointIn2D << " --> correct it (TODO)" << std::endl;
      }

      projectedContour->AddVertex( projectedPointIn2D, currentTimestep );
      iter++;
    }
  }

  return projectedContour;
}


mitk::ContourModel::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, Contour* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  mitk::Contour::PointsContainerIterator it = contourIn2D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn2D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return this->BackProjectContourFrom2DSlice(sliceGeometry, contour, false/*not used*/);
}

mitk::ContourModel::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, ContourModel* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  if ( !sliceGeometry || !contourIn2D ) return NULL;

  ContourModel::Pointer worldContour = ContourModel::New();
  worldContour->Initialize(*contourIn2D);

  int numberOfTimesteps = contourIn2D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
  ContourModel::VertexIterator iter = contourIn2D->Begin(currentTimestep);
  ContourModel::VertexIterator end = contourIn2D->End(currentTimestep);

  while( iter != end)
  {
    Point3D currentPointIn2D = (*iter)->Coordinates;

    Point3D worldPointIn3D;
    worldPointIn3D.Fill(0.0);
    sliceGeometry->IndexToWorld( currentPointIn2D, worldPointIn3D );
    //MITK_INFO << "index " << currentPointIn2D << " world " << worldPointIn3D << std::endl;

    worldContour->AddVertex( worldPointIn3D, currentTimestep );
    iter++;
  }
  }

  return worldContour;
}



void mitk::ContourUtils::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  mitk::Contour::PointsContainerIterator it = projectedContour->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = projectedContour->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  this->FillContourInSlice(contour, sliceImage, paintingPixelValue);
}


void mitk::ContourUtils::FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  this->FillContourInSlice(projectedContour, 0, sliceImage, paintingPixelValue);
}


void mitk::ContourUtils::FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, int paintingPixelValue )
{
  // 1. Use ipSegmentation to draw a filled(!) contour into a new 8 bit 2D image, which will later be copied back to the slice.
  //    We don't work on the "real" working data, because ipSegmentation would restrict us to 8 bit images

  // convert the projected contour into a ipSegmentation format
  mitkIpInt4_t* picContour = new mitkIpInt4_t[2 * projectedContour->GetNumberOfVertices(timeStep)];
  unsigned int index(0);
  ContourModel::VertexIterator iter = projectedContour->Begin(timeStep);
  ContourModel::VertexIterator end = projectedContour->End(timeStep);

  while( iter != end)
  {
    picContour[ 2 * index + 0 ] = static_cast<mitkIpInt4_t>( (*iter)->Coordinates[0] + 1.0 ); // +0.5 wahrscheinlich richtiger
    picContour[ 2 * index + 1 ] = static_cast<mitkIpInt4_t>( (*iter)->Coordinates[1] + 1.0 );
    //MITK_INFO << "mitk 2d [" << (*iter)[0] << ", " << (*iter)[1] << "]  pic [" << picContour[ 2*index+0] << ", " << picContour[ 2*index+1] << "]";
    iter++;
    index++;
  }

  assert( sliceImage->GetSliceData() );
  mitkIpPicDescriptor* originalPicSlice = mitkIpPicNew();
  CastToIpPicDescriptor( sliceImage, originalPicSlice);
  mitkIpPicDescriptor* picSlice = ipMITKSegmentationNew( originalPicSlice );
  ipMITKSegmentationClear( picSlice );

  assert( originalPicSlice && picSlice );

  // here comes the actual contour filling algorithm (from ipSegmentation/Graphics Gems)
  ipMITKSegmentationCombineRegion ( picSlice, picContour, projectedContour->GetNumberOfVertices(timeStep), NULL, IPSEGMENTATION_OR,  1); // set to 1

  delete[] picContour;

  // 2. Copy the filled contour to the working data slice
  //    copy all pixels that are non-zero to the original image (not caring for the actual type of the working image). perhaps make the replace value a parameter ( -> general painting tool ).
  //    make the pic slice an mitk/itk image (as little ipPic code as possible), call a templated method with accessbyitk, iterate over the original and the modified slice

  Image::Pointer ipsegmentationModifiedSlice = Image::New();
  ipsegmentationModifiedSlice->Initialize( CastToImageDescriptor( picSlice ) );
  ipsegmentationModifiedSlice->SetSlice( picSlice->data );

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
