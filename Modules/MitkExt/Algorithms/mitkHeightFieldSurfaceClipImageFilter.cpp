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

#include <vtkPolyData.h>
#include <vtkCellLocator.h>

#include <limits>


namespace mitk
{

HeightFieldSurfaceClipImageFilter::HeightFieldSurfaceClipImageFilter() 
: m_OutsideValue( 0.0 ),
  m_HeightFieldResolutionX( 512 ),
  m_HeightFieldResolutionY( 512 ),
  m_MaxHeight( 1024.0 )
{
  //this->SetNumberOfInputs(2);
  //this->SetNumberOfRequiredInputs(2);

  m_InputTimeSelector = ImageTimeSelector::New();
  m_OutputTimeSelector = ImageTimeSelector::New();
}


HeightFieldSurfaceClipImageFilter::~HeightFieldSurfaceClipImageFilter()
{
}


void HeightFieldSurfaceClipImageFilter::SetClippingSurface( 
  Surface *clippingSurface )
{
  if ( clippingSurface != m_ClippingSurface.GetPointer() )
  {
    m_ClippingSurface = clippingSurface;
    this->Modified();
  }
}


const Surface* HeightFieldSurfaceClipImageFilter::GetClippingSurface() const 
{
  return m_ClippingSurface;
}


void HeightFieldSurfaceClipImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  Image *output = this->GetOutput();
  Image *input = const_cast< Image * > ( this->GetInput() );
  if ( !output->IsInitialized() || m_ClippingSurface.IsNull() )
  {
    return;
  }

  input->SetRequestedRegionToLargestPossibleRegion();

  GenerateTimeInInputRegion( output, input );
}


void HeightFieldSurfaceClipImageFilter::GenerateOutputInformation()
{
  Image::ConstPointer input = this->GetInput();
  Image::Pointer output = this->GetOutput();

  if ( output->IsInitialized()
    && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()) )
  {
    return;
  }

  itkDebugMacro(<<"GenerateOutputInformation()");

  unsigned int i;
  unsigned int *tmpDimensions = new unsigned int[input->GetDimension()];

  for ( i = 0; i < input->GetDimension(); ++i )
  {
    tmpDimensions[i] = input->GetDimension( i );
  }

  output->Initialize( input->GetPixelType(),
    input->GetDimension(),
    tmpDimensions,
    input->GetNumberOfChannels() );

  delete[] tmpDimensions;

  output->SetGeometry( 
    static_cast< Geometry3D * >( input->GetGeometry()->Clone().GetPointer() ) );

  output->SetPropertyList( input->GetPropertyList()->Clone() );

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

  typedef itk::ImageRegionConstIteratorWithIndex< ItkInputImageType > ItkInputImageIteratorType;
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

  typename ItkOutputImageType::PixelType outsideValue = clipImageFilter->m_OutsideValue;


  // Get bounds of clipping data
  clippingPolyData->ComputeBounds();
  vtkFloatingPointType *bounds = clippingPolyData->GetBounds();

  double x0 = bounds[0];
  double xWidth = bounds[1] - bounds[0];
  double y0 = bounds[2];
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
  LOG_INFO << "Calculating Height Field..." << std::endl;
  for ( unsigned int y = 0; y < m_HeightFieldResolutionY; ++y )
  {
    for ( unsigned int x = 0; x < m_HeightFieldResolutionX; ++x )
    {
      vtkFloatingPointType p0[3], p1[3], surfacePoint[3], pcoords[3];
      p0[0] = x0 + xWidth * x / (double) m_HeightFieldResolutionX;
      p0[1] = y0 + xWidth * y / (double) m_HeightFieldResolutionY;
      p0[2] = -m_MaxHeight;

      p1[0] = p0[0];
      p1[1] = p0[1];
      p1[2] = m_MaxHeight;

      double t, distance;
      int subId;
      if ( cellLocator->IntersectWithLine( p0, p1, 0.0, t, surfacePoint, pcoords, subId ) )
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
  LOG_INFO << "Performing clipping..." << std::endl;
  for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
  {
    if ( (typename ItkOutputImageType::PixelType)inputIt.Get() == outsideValue )
    {
      outputIt.Set( outsideValue );
    }
    else
    {
      Point3D imagePoint;
      imagePoint[0] = inputIt.GetIndex()[0];
      imagePoint[1] = inputIt.GetIndex()[1];
      imagePoint[2] = inputIt.GetIndex()[2];

      Point3D planePoint = imageToPlaneTransform->TransformPoint( imagePoint );

      unsigned int x = (unsigned int) m_HeightFieldResolutionX * (planePoint[0] - x0) / xWidth;
      unsigned int y = (unsigned int) m_HeightFieldResolutionY * (planePoint[1] - y0) / yWidth;

      bool clip;
      if ( (x < 0) || (x >= m_HeightFieldResolutionX)
        || (y < 0) || (y >= m_HeightFieldResolutionY) )
      {
        clip = true;        
      }
      else
      {
        if ( heightField[y * m_HeightFieldResolutionX + x] - planePoint[2] < 0 )
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
        outputIt.Set( 0 );
      }
      else
      {
        outputIt.Set( 1 );
      }
    }
  }

  LOG_INFO << "DONE!" << std::endl;


  // Clean-up
  cellLocator->Delete();
}


void HeightFieldSurfaceClipImageFilter::GenerateData()
{
  Image::ConstPointer input = this->GetInput();
  Image::Pointer output = this->GetOutput();

  LOG_INFO << "Clipping: Start\n";

  if ( !output->IsInitialized() || m_ClippingSurface.IsNull() )
    return;

  const Geometry2D *clippingGeometryOfCurrentTimeStep = NULL;

  m_InputTimeSelector->SetInput( input );
  m_OutputTimeSelector->SetInput( output );

  Image::RegionType outputRegion = output->GetRequestedRegion();
  const TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();
  const TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
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
    m_ClippingSurface->GetGeometry( t )->GetIndexToWorldTransform()
      ->GetInverse( planeWorldToIndexTransform );
    
    AffineTransform3D *imageToPlaneTransform =
      inputTimeGeometry->GetGeometry3D( t )->GetIndexToWorldTransform();
    imageToPlaneTransform->Compose( planeWorldToIndexTransform );

    LOG_INFO << "Accessing ITK function...\n";
    AccessByItk_3( 
      m_InputTimeSelector->GetOutput(),
      _InternalComputeClippedImage,
      this,
      m_ClippingSurface->GetVtkPolyData( t ),
      imageToPlaneTransform );
  }

  m_TimeOfHeaderInitialization.Modified();
}

} // namespace