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

#include "mitkExtractImageFilter.h"
#include "mitkImageCast.h"
#include "mitkPlaneGeometry.h"
#include "mitkITKImageImport.h"
#include "mitkImageTimeSelector.h"

#include <itkExtractImageFilter.h>

mitk::ExtractImageFilter::ExtractImageFilter()
:m_SliceIndex(0),
 m_SliceDimension(0),
 m_TimeStep(0)
{
}

mitk::ExtractImageFilter::~ExtractImageFilter()
{
}

void mitk::ExtractImageFilter::GenerateData()
{
   Image::ConstPointer input = ImageToImageFilter::GetInput(0);

   if ( (input->GetDimension() > 4) || (input->GetDimension() < 2) )
   {
     LOG_ERROR << "mitk::ExtractImageFilter:GenerateData works only with 3D and 3D+t images, sorry." << std::endl;
     itkExceptionMacro("mitk::ExtractImageFilter works only with 3D and 3D+t images, sorry.");
     return;
   }
   else if (input->GetDimension() == 4)
   {
     ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
     timeSelector->SetInput( input );
     timeSelector->SetTimeNr( m_TimeStep );
     timeSelector->UpdateLargestPossibleRegion();
     input = timeSelector->GetOutput();
   }
   else if (input->GetDimension() == 2)
   {
     Image::Pointer resultImage = ImageToImageFilter::GetOutput();
     resultImage = const_cast<Image*>(input.GetPointer());
     ImageToImageFilter::SetNthOutput( 0, resultImage );
     return;
   }

  if ( m_SliceDimension >= input->GetDimension() )
  {
    LOG_ERROR << "mitk::ExtractImageFilter:GenerateData  m_SliceDimension == " << m_SliceDimension << " makes no sense with an " << input->GetDimension() << "D image." << std::endl;
    itkExceptionMacro("This is not a sensible value for m_SliceDimension.");
    return;
  }

   AccessFixedDimensionByItk( input, ItkImageProcessing, 3 );

  // set a nice geometry for display and point transformations
  Geometry3D* inputImageGeometry = ImageToImageFilter::GetInput(0)->GetGeometry();
  if (!inputImageGeometry)
  {
    LOG_ERROR << "In ExtractImageFilter::ItkImageProcessing: Input image has no geometry!" << std::endl;
    return;
  }

  PlaneGeometry::PlaneOrientation orientation = PlaneGeometry::Transversal;

  switch ( m_SliceDimension )
  {
    default:
    case 2: 
      orientation = PlaneGeometry::Transversal;
      break;
    case 1: 
      orientation = PlaneGeometry::Frontal;
      break;
    case 0: 
      orientation = PlaneGeometry::Sagittal;
      break;
   }
 
  PlaneGeometry::Pointer planeGeometry = PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( inputImageGeometry, orientation, (ScalarType)m_SliceIndex + 0.5 , true, false );
  Image::Pointer resultImage = ImageToImageFilter::GetOutput();
  resultImage->SetGeometry( planeGeometry );
}
    
template<typename TPixel, unsigned int VImageDimension>
void mitk::ExtractImageFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* itkImage )
{
  // use the itk::ExtractImageFilter to get a 2D image
  typedef itk::Image< TPixel, VImageDimension >   ImageType3D;
  typedef itk::Image< TPixel, VImageDimension-1 > ImageType2D;

  typename ImageType3D::RegionType inSliceRegion = itkImage->GetLargestPossibleRegion();
  
  inSliceRegion.SetSize( m_SliceDimension, 0 );

  typedef itk::ExtractImageFilter<ImageType3D, ImageType2D> ExtractImageFilterType;

  typename ExtractImageFilterType::Pointer sliceExtractor = ExtractImageFilterType::New();
  sliceExtractor->SetInput( itkImage );
    
  inSliceRegion.SetIndex( m_SliceDimension, m_SliceIndex );

  sliceExtractor->SetExtractionRegion( inSliceRegion );

  // calculate the output
  sliceExtractor->UpdateLargestPossibleRegion();

  typename ImageType2D::Pointer slice = sliceExtractor->GetOutput();
  // possible bug in itk::ExtractImageFilter: or in CastToItkImage ?
  // direction maxtrix is wrong/broken/not working with mitk::Image::InitializeByItkImage
  // solution here: we overwrite it with an unity matrix
  // correct image direction only if ITK version < 3.10
#if (ITK_VERSION_MAJOR == 3 && ITK_VERSION_MINOR < 10) || ITK_VERSION_MAJOR < 3
  typename ImageType2D::DirectionType imageDirection;
  imageDirection.SetIdentity();
  slice->SetDirection(imageDirection);
#endif

  // re-import to MITK
  Image::Pointer resultImage = ImageToImageFilter::GetOutput();
  GrabItkImageMemory(slice, resultImage, NULL, false);
}

/*
 * What is the input requested region that is required to produce the output
 * requested region? By default, the largest possible region is always
 * required but this is overridden in many subclasses. For instance, for an
 * image processing filter where an output pixel is a simple function of an
 * input pixel, the input requested region will be set to the output
 * requested region. For an image processing filter where an output pixel is
 * a function of the pixels in a neighborhood of an input pixel, then the
 * input requested region will need to be larger than the output requested
 * region (to avoid introducing artificial boundary conditions). This
 * function should never request an input region that is outside the the
 * input largest possible region (i.e. implementations of this method should
 * crop the input requested region at the boundaries of the input largest
 * possible region). 
 */
void mitk::ExtractImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  ImageToImageFilter::InputImagePointer input = const_cast< ImageToImageFilter::InputImageType* > ( this->GetInput() );
  Image::Pointer output = this->GetOutput();

  if (input->GetDimension() == 2)
  {
    input->SetRequestedRegionToLargestPossibleRegion();
    return;
  }

  Image::RegionType requestedRegion;
  requestedRegion = output->GetRequestedRegion();
  requestedRegion.SetIndex(0, 0);
  requestedRegion.SetIndex(1, 0);
  requestedRegion.SetIndex(2, 0);
  requestedRegion.SetSize(0, input->GetDimension(0));
  requestedRegion.SetSize(1, input->GetDimension(1));
  requestedRegion.SetSize(2, input->GetDimension(2));

  requestedRegion.SetIndex( m_SliceDimension, m_SliceIndex ); // only one slice needed
  requestedRegion.SetSize( m_SliceDimension, 1 );

  input->SetRequestedRegion( &requestedRegion );
}

/*
 * Generate the information decribing the output data. The default
 * implementation of this method will copy information from the input to the
 * output. A filter may override this method if its output will have different
 * information than its input. For instance, a filter that shrinks an image will
 * need to provide an implementation for this method that changes the spacing of
 * the pixels. Such filters should call their superclass' implementation of this
 * method prior to changing the information values they need (i.e.
 * GenerateOutputInformation() should call
 * Superclass::GenerateOutputInformation() prior to changing the information.
 */
void mitk::ExtractImageFilter::GenerateOutputInformation()
{
 Image::Pointer output = this->GetOutput();
 Image::ConstPointer input = this->GetInput();
 if (input.IsNull()) return;

 if ( m_SliceDimension >= input->GetDimension() && input->GetDimension() != 2 )
 {
   LOG_ERROR << "mitk::ExtractImageFilter:GenerateOutputInformation  m_SliceDimension == " << m_SliceDimension << " makes no sense with an " << input->GetDimension() << "D image." << std::endl;
   itkExceptionMacro("This is not a sensible value for m_SliceDimension.");
   return;
 }

 unsigned int sliceDimension( m_SliceDimension );
 if ( input->GetDimension() == 2)
 {
   sliceDimension = 2;
 }

  unsigned int tmpDimensions[2];

  switch ( sliceDimension )
  {
    default:
    case 2: 
      // orientation = PlaneGeometry::Transversal;
      tmpDimensions[0] = input->GetDimension(0);
      tmpDimensions[1] = input->GetDimension(1);
      break;
    case 1: 
      // orientation = PlaneGeometry::Frontal;
      tmpDimensions[0] = input->GetDimension(0);
      tmpDimensions[1] = input->GetDimension(2);
      break;
    case 0: 
      // orientation = PlaneGeometry::Sagittal;
      tmpDimensions[0] = input->GetDimension(1);
      tmpDimensions[1] = input->GetDimension(2);
      break;
  }

  output->Initialize(input->GetPixelType(), 2, tmpDimensions, 1 /*input->GetNumberOfChannels()*/);

  // initialize the spacing of the output
/*
  Vector3D spacing = input->GetSlicedGeometry()->GetSpacing();
  if(input->GetDimension()>=2)
    spacing[2]=spacing[1];
  else
    spacing[2] = 1.0;
  output->GetSlicedGeometry()->SetSpacing(spacing);
*/

  output->SetPropertyList(input->GetPropertyList()->Clone());
}


