/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 15:59:04 +0200 (Do, 18 Jun 2009) $
Version:   $Revision: 17786 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkHeightFieldSurfaceClipImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkTimeHelper.h"
#include "mitkProperties.h"

#include "mitkImageToItk.h"
#include "mitkImageAccessByItk.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageSliceConstIteratorWithIndex.h>

#include <vtkPolyData.h>
#include <vtkCellLocator.h>

#include <limits>


namespace mitk
{

HeightFieldSurfaceClipImageFilter::HeightFieldSurfaceClipImageFilter() 
: m_ClippingMode( CLIPPING_MODE_CONSTANT ),
  m_ClippingConstant( 0.0 ),
  m_MultiplicationFactor( 2.0 ),
  m_HeightFieldResolutionX( 256 ),
  m_HeightFieldResolutionY( 256 ),
  m_MaxHeight( 1024.0 )
{
  this->SetNumberOfInputs(2);
  this->SetNumberOfRequiredInputs(2);

  m_InputTimeSelector = ImageTimeSelector::New();
  m_OutputTimeSelector = ImageTimeSelector::New();
}


HeightFieldSurfaceClipImageFilter::~HeightFieldSurfaceClipImageFilter()
{
}


void HeightFieldSurfaceClipImageFilter::SetClippingSurface( 
  Surface *clippingSurface )
{
  this->SetNthInput( 1, clippingSurface );
}


const Surface* HeightFieldSurfaceClipImageFilter::GetClippingSurface() const 
{
  return dynamic_cast< const Surface * >( itk::ProcessObject::GetInput( 1 ) );
}


void HeightFieldSurfaceClipImageFilter::SetClippingMode( int mode )
{
  m_ClippingMode = mode;
}


int HeightFieldSurfaceClipImageFilter::GetClippingMode()
{
  return m_ClippingMode;
}


void HeightFieldSurfaceClipImageFilter::SetClippingModeToConstant()
{
  m_ClippingMode = CLIPPING_MODE_CONSTANT;
}


void HeightFieldSurfaceClipImageFilter::SetClippingModeToMultiplyByFactor()
{
  m_ClippingMode = CLIPPING_MODE_MULTIPLYBYFACTOR;
}


void HeightFieldSurfaceClipImageFilter::GenerateInputRequestedRegion()
{
  Image *outputImage = this->GetOutput();
  Image *inputImage = const_cast< Image * >( this->GetInput( 0 ) );
  const Surface *inputSurface = dynamic_cast< const Surface * >( this->GetInput( 1 ) );

  if ( !outputImage->IsInitialized() || inputSurface == NULL )
  {
    return;
  }

  inputImage->SetRequestedRegionToLargestPossibleRegion();

  GenerateTimeInInputRegion( outputImage, inputImage );
}


void HeightFieldSurfaceClipImageFilter::GenerateOutputInformation()
{
  const Image *inputImage = this->GetInput( 0 );
  Image *outputImage = this->GetOutput();

  if ( outputImage->IsInitialized()
    && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()) )
  {
    return;
  }

  itkDebugMacro(<<"GenerateOutputInformation()");

  unsigned int i;
  unsigned int *tmpDimensions = new unsigned int[inputImage->GetDimension()];

  for ( i = 0; i < inputImage->GetDimension(); ++i )
  {
    tmpDimensions[i] = inputImage->GetDimension( i );
  }

  outputImage->Initialize( inputImage->GetPixelType(),
    inputImage->GetDimension(),
    tmpDimensions,
    inputImage->GetNumberOfChannels() );

  delete[] tmpDimensions;

  outputImage->SetGeometry( 
    static_cast< Geometry3D * >( inputImage->GetGeometry()->Clone().GetPointer() ) );

  outputImage->SetPropertyList( inputImage->GetPropertyList()->Clone() );

  m_TimeOfHeaderInitialization.Modified();
}


template < typename TPixel, unsigned int VImageDimension >
void HeightFieldSurfaceClipImageFilter::_InternalComputeClippedImage(
  itk::Image< TPixel, VImageDimension > *inputItkImage, 
  HeightFieldSurfaceClipImageFilter *clipImageFilter, 
  vtkPolyData *clippingPolyData,
  AffineTransform3D *imageToPlaneTransform )
{
  typedef itk::Image< TPixel, VImageDimension > ItkInputImageType;
  typedef itk::Image< TPixel, VImageDimension > ItkOutputImageType;

  typedef itk::ImageSliceConstIteratorWithIndex< ItkInputImageType > ItkInputImageIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< ItkOutputImageType > ItkOutputImageIteratorType;

  typename ImageToItk<ItkOutputImageType >::Pointer outputimagetoitk = 
    ImageToItk<ItkOutputImageType>::New();
  outputimagetoitk->SetInput( clipImageFilter->m_OutputTimeSelector->GetOutput() );
  outputimagetoitk->Update();

  typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

  
  // create the iterators
  typename ItkInputImageType::RegionType inputRegionOfInterest = 
    inputItkImage->GetLargestPossibleRegion();
  ItkInputImageIteratorType  inputIt( inputItkImage, inputRegionOfInterest );
  ItkOutputImageIteratorType outputIt( outputItkImage, inputRegionOfInterest );

  // Get bounds of clipping data
  clippingPolyData->ComputeBounds();
  vtkFloatingPointType *bounds = clippingPolyData->GetBounds();

  double xWidth = bounds[1] - bounds[0];
  double yWidth = bounds[3] - bounds[2];

  // Create vtkCellLocator for clipping poly data
  vtkCellLocator *cellLocator = vtkCellLocator::New();
  cellLocator->SetDataSet( clippingPolyData );
  cellLocator->CacheCellBoundsOn();
  cellLocator->AutomaticOn();
  cellLocator->BuildLocator();

  // Allocate memory for 2D image to hold the height field generated by
  // projecting the clipping data onto the plane
  double *heightField = new double[m_HeightFieldResolutionX * m_HeightFieldResolutionY];

  // Walk through height field and for each entry calculate height of the
  // clipping poly data at this point by means of vtkCellLocator. The
  // clipping data x/y bounds are used for converting from poly data space to
  // image (height-field) space.
  MITK_INFO << "Calculating Height Field..." << std::endl;
  for ( unsigned int y = 0; y < m_HeightFieldResolutionY; ++y )
  {
    for ( unsigned int x = 0; x < m_HeightFieldResolutionX; ++x )
    {
      vtkFloatingPointType p0[3], p1[3], surfacePoint[3], pcoords[3];
      p0[0] = bounds[0] + xWidth * x / (double) m_HeightFieldResolutionX;
      p0[1] = bounds[2] + yWidth * y / (double) m_HeightFieldResolutionY;
      p0[2] = -m_MaxHeight;

      p1[0] = p0[0];
      p1[1] = p0[1];
      p1[2] = m_MaxHeight;

      double t, distance;
      int subId;
      if ( cellLocator->IntersectWithLine( p0, p1, 0.1, t, surfacePoint, pcoords, subId ) )
      {
        distance = (2.0 * t - 1.0) * m_MaxHeight;
      }
      else
      {
        distance = -65536.0;
      }
      heightField[y * m_HeightFieldResolutionX + x] = distance;
    }
  }


  // Walk through entire input image and for each point determine its distance
  // from the x/y plane.
  MITK_INFO << "Performing clipping..." << std::endl;

  TPixel factor = static_cast< TPixel >( clipImageFilter->m_MultiplicationFactor );
  TPixel clippingConstant = clipImageFilter->m_ClippingConstant;

  inputIt.SetFirstDirection( 0 );
  inputIt.SetSecondDirection( 1 );

  for ( inputIt.GoToBegin(), outputIt.GoToBegin();
        !inputIt.IsAtEnd(); 
        inputIt.NextSlice() )
  {
    for ( ; !inputIt.IsAtEndOfSlice(); inputIt.NextLine() )
    {
      Point3D imageP0, planeP0;
      imageP0[0] = inputIt.GetIndex()[0];
      imageP0[1] = inputIt.GetIndex()[1];
      imageP0[2] = inputIt.GetIndex()[2];
      planeP0 = imageToPlaneTransform->TransformPoint( imageP0 );

      Point3D imageP1, planeP1;
      imageP1[0] = imageP0[0] + inputRegionOfInterest.GetSize( 0 );
      imageP1[1] = imageP0[1];
      imageP1[2] = imageP0[2];
      planeP1 = imageToPlaneTransform->TransformPoint( imageP1 );

      Vector3D step = (planeP1 - planeP0) / (double) inputRegionOfInterest.GetSize( 0 );


      for ( ; !inputIt.IsAtEndOfLine(); ++inputIt, ++outputIt, planeP0 += step )
      {
        if ( (clipImageFilter->m_ClippingMode == CLIPPING_MODE_CONSTANT)
          && ((TPixel)inputIt.Get() == clippingConstant ) )
        {
          outputIt.Set( clippingConstant );
        }
        else
        {
          int x0 = (int) ((double)(m_HeightFieldResolutionX) * (planeP0[0] - bounds[0]) / xWidth);
          int y0 = (int) ((double)(m_HeightFieldResolutionY) * (planeP0[1] - bounds[2]) / yWidth);

          bool clip;
          if ( (x0 < 0) || (x0 >= (int)m_HeightFieldResolutionX)
            || (y0 < 0) || (y0 >= (int)m_HeightFieldResolutionY) )
          {
            clip = true;
          }
          else
          {
            // Calculate bilinearly interpolated height field value at plane point
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            if ( x1 >= (int)m_HeightFieldResolutionX ) { x1 = x0; }
            if ( y1 >= (int)m_HeightFieldResolutionY ) { y1 = y0; }

            ScalarType q00, q01, q10, q11;
            q00 = heightField[y0 * m_HeightFieldResolutionX + x0];
            q01 = heightField[y0 * m_HeightFieldResolutionX + x1];
            q10 = heightField[y1 * m_HeightFieldResolutionX + x0];
            q11 = heightField[y1 * m_HeightFieldResolutionX + x1];

            ScalarType q =
                q00 * ((double) x1 - planeP0[0]) * ((double) y1 - planeP0[1])
              + q01 * (planeP0[0] - (double) x0) * ((double) y1 - planeP0[1])
              + q10 * ((double) x1 - planeP0[0]) * (planeP0[1] - (double) y0)
              + q11 * (planeP0[0] - (double) x0) * (planeP0[1] - (double) y0);

            if ( q - planeP0[2] < 0 )
            {
              clip = true;
            }
            else
            {
              clip = false;
            }
          }

          if ( clip )
          {
            if ( clipImageFilter->m_ClippingMode == CLIPPING_MODE_CONSTANT )
            {
              outputIt.Set( clipImageFilter->m_ClippingConstant );
            }
            else if ( clipImageFilter->m_ClippingMode == CLIPPING_MODE_MULTIPLYBYFACTOR )
            {
              outputIt.Set( inputIt.Get() * factor );
            }
          }
          else
          {
            outputIt.Set( inputIt.Get() );
          }
        }
      }
    }
  }

  MITK_INFO << "DONE!" << std::endl;


  // Clean-up
  cellLocator->Delete();
}


void HeightFieldSurfaceClipImageFilter::GenerateData()
{
  const Image *inputImage = this->GetInput( 0 );
  Surface *inputSurface = const_cast< Surface * >(
    dynamic_cast< Surface * >( itk::ProcessObject::GetInput( 1 ) ) );

  const Image *outputImage = this->GetOutput();

  MITK_INFO << "Clipping: Start\n";

  if ( !outputImage->IsInitialized() || inputSurface == NULL )
    return;


  //const Geometry2D *clippingGeometryOfCurrentTimeStep = NULL;

  m_InputTimeSelector->SetInput( inputImage );
  m_OutputTimeSelector->SetInput( outputImage );

  Image::RegionType outputRegion = outputImage->GetRequestedRegion();
  const TimeSlicedGeometry *outputTimeGeometry = outputImage->GetTimeSlicedGeometry();
  const TimeSlicedGeometry *inputTimeGeometry = inputImage->GetTimeSlicedGeometry();
  ScalarType timeInMS;

  int timestep = 0;
  int tstart = outputRegion.GetIndex( 3 );
  int tmax = tstart + outputRegion.GetSize( 3 );

  int t;
  for( t = tstart; t < tmax; ++t )
  {
    timeInMS = outputTimeGeometry->TimeStepToMS( t );
    timestep = inputTimeGeometry->MSToTimeStep( timeInMS );

    m_InputTimeSelector->SetTimeNr( timestep );
    m_InputTimeSelector->UpdateLargestPossibleRegion();
    m_OutputTimeSelector->SetTimeNr( t );
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    // Compose IndexToWorld transform of image with WorldToIndexTransform of
    // clipping data for conversion from image index space to plane index space
    AffineTransform3D::Pointer planeWorldToIndexTransform = AffineTransform3D::New();
    inputSurface->GetGeometry( t )->GetIndexToWorldTransform()
      ->GetInverse( planeWorldToIndexTransform );
   
    AffineTransform3D::Pointer imageToPlaneTransform =
      AffineTransform3D::New();
    imageToPlaneTransform->SetIdentity();

    imageToPlaneTransform->Compose(
      inputTimeGeometry->GetGeometry3D( t )->GetIndexToWorldTransform() );
    imageToPlaneTransform->Compose( planeWorldToIndexTransform );

    MITK_INFO << "Accessing ITK function...\n";
    AccessByItk_3( 
      m_InputTimeSelector->GetOutput(),
      _InternalComputeClippedImage,
      this,
      inputSurface->GetVtkPolyData( t ),
      imageToPlaneTransform );
  }

  m_TimeOfHeaderInitialization.Modified();
}

} // namespace
