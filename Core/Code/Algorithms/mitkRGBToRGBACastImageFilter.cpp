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


#include "mitkRGBToRGBACastImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageToItk.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageIOBase.h>
#include <itkRGBAPixel.h>


mitk::RGBToRGBACastImageFilter::RGBToRGBACastImageFilter()
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_InputTimeSelector  = mitk::ImageTimeSelector::New();
  m_OutputTimeSelector = mitk::ImageTimeSelector::New();
}


mitk::RGBToRGBACastImageFilter::~RGBToRGBACastImageFilter()
{
}


bool mitk::RGBToRGBACastImageFilter::IsRGBImage( const mitk::Image *image )
{
  const mitk::PixelType &inputPixelType = image->GetPixelType();

  if ( (inputPixelType.GetPixelType() == itk::ImageIOBase::RGB )
    && ( (inputPixelType.GetComponentType() == itk::ImageIOBase::UCHAR )
      || (inputPixelType.GetComponentType() == itk::ImageIOBase::USHORT )
      || (inputPixelType.GetComponentType() == itk::ImageIOBase::FLOAT )
      || (inputPixelType.GetComponentType() == itk::ImageIOBase::DOUBLE )
       )
     )
  {
    return true;
  }

  return false;
}


void mitk::RGBToRGBACastImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* output = this->GetOutput();
  mitk::Image* input = const_cast< mitk::Image * > ( this->GetInput() );
  if ( !output->IsInitialized() )
  {
    return;
  }

  input->SetRequestedRegionToLargestPossibleRegion();

  //GenerateTimeInInputRegion(output, input);
}

void mitk::RGBToRGBACastImageFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<<"GenerateOutputInformation()");

  // Initialize RGBA output with same pixel type as input image
  const mitk::PixelType &inputPixelType = input->GetPixelType();

  typedef itk::Image< UCRGBPixelType > UCRGBItkImageType;
  typedef itk::Image< USRGBPixelType > USRGBItkImageType;
  typedef itk::Image< FloatRGBPixelType > FloatCRGBItkImageType;
  typedef itk::Image< DoubleRGBPixelType > DoubleRGBItkImageType;

  if ( inputPixelType == mitk::MakePixelType< UCRGBItkImageType>() )
  {
    const mitk::PixelType refPtype = MakePixelType<UCRGBItkImageType>();
    output->Initialize( refPtype, *input->GetTimeSlicedGeometry() );

  }
  else if ( inputPixelType == mitk::MakePixelType< USRGBItkImageType>( ) )
  {
    const mitk::PixelType refPtype = MakePixelType<USRGBItkImageType>();
    output->Initialize( refPtype, *input->GetTimeSlicedGeometry() );
  }
  else if ( inputPixelType == mitk::MakePixelType< FloatCRGBItkImageType>( ) )
  {
    const mitk::PixelType refPtype = MakePixelType<FloatCRGBItkImageType>();
    output->Initialize( refPtype, *input->GetTimeSlicedGeometry() );
  }
  else if ( inputPixelType == mitk::MakePixelType< DoubleRGBItkImageType>( ) )
  {
    const mitk::PixelType refPtype = MakePixelType<DoubleRGBItkImageType>();
    output->Initialize( refPtype, *input->GetTimeSlicedGeometry() );
  }

  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}


void mitk::RGBToRGBACastImageFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if( !output->IsInitialized() )
  {
    return;
  }

  m_InputTimeSelector->SetInput(input);
  m_OutputTimeSelector->SetInput(this->GetOutput());

  mitk::Image::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  ScalarType timeInMS;

  int timestep=0;
  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3);

  int t;
  for(t=tstart;t<tmax;++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToMS( t );

    timestep = inputTimeGeometry->MSToTimeStep( timeInMS );

    m_InputTimeSelector->SetTimeNr(timestep);
    m_InputTimeSelector->UpdateLargestPossibleRegion();
    m_OutputTimeSelector->SetTimeNr(t);
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    mitk::Image *image = m_InputTimeSelector->GetOutput();

    const mitk::PixelType &pixelType = image->GetPixelType();

    // Check if the pixel type is supported
    if ( pixelType == MakePixelType< itk::Image<UCRGBPixelType> >() )
    {
      AccessFixedPixelTypeByItk_2( image, InternalCast, (UCRGBPixelType), this, 255 );
    }
    else if ( pixelType == MakePixelType< itk::Image< USRGBPixelType> >() )
    {
      AccessFixedPixelTypeByItk_2( image, InternalCast, (USRGBPixelType), this, 65535 );
    }
    else if ( pixelType == MakePixelType< itk::Image< FloatRGBPixelType> >() )
    {
      AccessFixedPixelTypeByItk_2( image, InternalCast, (FloatRGBPixelType), this, 1.0 );
    }
    else if ( pixelType == MakePixelType< itk::Image< DoubleRGBPixelType> >() )
    {
      AccessFixedPixelTypeByItk_2( image, InternalCast, (DoubleRGBPixelType), this, 1.0 );
    }
    else
    {
      // Otherwise, write warning and graft input to output

      // ...TBD...
    }

  }

  m_TimeOfHeaderInitialization.Modified();
}


template < typename TPixel, unsigned int VImageDimension >
void mitk::RGBToRGBACastImageFilter::InternalCast(
  itk::Image< TPixel, VImageDimension > *inputItkImage,
  mitk::RGBToRGBACastImageFilter *addComponentFilter,
  typename TPixel::ComponentType defaultAlpha )
{
  typedef TPixel InputPixelType;
  typedef itk::RGBAPixel< typename TPixel::ComponentType > OutputPixelType;
  typedef itk::Image< InputPixelType, VImageDimension > InputImageType;
  typedef itk::Image< OutputPixelType, VImageDimension > OutputImageType;

  typedef itk::ImageRegionConstIterator< InputImageType > InputImageIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< OutputImageType > OutputImageIteratorType;

  typename mitk::ImageToItk< OutputImageType >::Pointer outputimagetoitk =
    mitk::ImageToItk< OutputImageType >::New();
  outputimagetoitk->SetInput(addComponentFilter->m_OutputTimeSelector->GetOutput());
  outputimagetoitk->Update();
  typename OutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

  // create the iterators
  typename InputImageType::RegionType inputRegionOfInterest =
    inputItkImage->GetLargestPossibleRegion();
  InputImageIteratorType  inputIt( inputItkImage, inputRegionOfInterest );
  OutputImageIteratorType outputIt( outputItkImage, inputRegionOfInterest );

  for ( inputIt.GoToBegin(), outputIt.GoToBegin();
        !inputIt.IsAtEnd();
        ++inputIt, ++outputIt )
  {
    typename InputPixelType::Iterator pixelInputIt = inputIt.Get().Begin();
    typename OutputPixelType::Iterator pixelOutputIt = outputIt.Get().Begin();

    *pixelOutputIt++ = *pixelInputIt++;
    *pixelOutputIt++ = *pixelInputIt++;
    *pixelOutputIt++ = *pixelInputIt++;
    *pixelOutputIt = defaultAlpha;
  }
}
