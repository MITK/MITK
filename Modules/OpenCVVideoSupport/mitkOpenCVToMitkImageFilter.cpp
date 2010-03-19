/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-14 14:20:26 +0100 (Do, 14 Jan 2010) $
Version:   $Revision: 21047 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkOpenCVToMitkImageFilter.h"

#include <itkImportImageFilter.h>
#include <itkRGBPixel.h>
#include <mitkITKImageImport.txx>

mitk::OpenCVToMitkImageFilter::OpenCVToMitkImageFilter()
: m_OpenCVImage(0)
{
}

mitk::OpenCVToMitkImageFilter::~OpenCVToMitkImageFilter()
{
}

void mitk::OpenCVToMitkImageFilter::GenerateData()
{
  if(m_OpenCVImage == 0)
  {
    MITK_WARN << "Cannot not start filter. OpenCV Image not set.";
    return;
  }

  // convert to rgb image color space
  IplImage* rgbOpenCVImage = cvCreateImage( cvSize( m_OpenCVImage->width, m_OpenCVImage->height )
    , m_OpenCVImage->depth, m_OpenCVImage->nChannels );

  if( m_OpenCVImage->nChannels == 3)
    cvCvtColor( m_OpenCVImage, rgbOpenCVImage,  CV_BGR2RGB );

  // now convert rgb image
  if( m_OpenCVImage->depth == IPL_DEPTH_8S && m_OpenCVImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< char, 2>( m_OpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_8U && m_OpenCVImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< unsigned char, 2>( m_OpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_8U && m_OpenCVImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< UCRGBPixelType, 2>( rgbOpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_16U && m_OpenCVImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< unsigned short, 2>( m_OpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_16U && m_OpenCVImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< USRGBPixelType, 2>( rgbOpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_32F && m_OpenCVImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< float, 2>( m_OpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_32F && m_OpenCVImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< FloatRGBPixelType , 2>( rgbOpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_64F && m_OpenCVImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< double, 2>( m_OpenCVImage );

  else if( m_OpenCVImage->depth == IPL_DEPTH_64F && m_OpenCVImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< DoubleRGBPixelType , 2>( rgbOpenCVImage );
}

mitk::ImageSource::DataObjectPointer mitk::OpenCVToMitkImageFilter::MakeOutput( unsigned int idx )
{
  return Superclass::MakeOutput(idx);
}

mitk::ImageSource::OutputImageType* mitk::OpenCVToMitkImageFilter::GetOutput( unsigned int idx )
{
  return m_Image;
}

/********************************************
* Converting from OpenCV image to ITK Image
*********************************************/
template <typename TPixel, unsigned int VImageDimension>
mitk::Image::Pointer mitk::OpenCVToMitkImageFilter::ConvertIplToMitkImage( const IplImage * input, bool copyBuffer )
{
  mitk::Image::Pointer mitkImage(0);

  typedef itk::Image< TPixel, VImageDimension > ItkImage;

  typedef itk::ImportImageFilter< TPixel, VImageDimension >  ImportFilterType;
  ImportFilterType::Pointer importFilter = ImportFilterType::New();

  ImportFilterType::SizeType  size;

  size[0]  = input->width;
  size[1]  = input->height;

  ImportFilterType::IndexType start;
  start.Fill( 0 );

  ImportFilterType::RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );

  importFilter->SetRegion( region );


  double origin[ VImageDimension ];
  origin[0] = 0.0;    // X coordinate
  origin[1] = 0.0;    // Y coordinate

  importFilter->SetOrigin( origin );


  double spacing[ VImageDimension ];
  spacing[0] = 1.0;    // along X direction
  spacing[1] = 1.0;    // along Y direction

  importFilter->SetSpacing( spacing );


  const unsigned int numberOfPixels = size[0] * size[1];
  const unsigned int numberOfBytes = numberOfPixels * sizeof( TPixel );

  if( copyBuffer )
  {
    const bool importImageFilterWillOwnTheBuffer = false;

    TPixel * localBuffer = new TPixel[numberOfPixels];

    memcpy(localBuffer, input->imageData, numberOfBytes);

    importFilter->SetImportPointer( localBuffer, numberOfPixels,
      importImageFilterWillOwnTheBuffer );
  }
  else
  {
    const bool importImageFilterWillOwnTheBuffer = false;

    TPixel * localBuffer = reinterpret_cast< TPixel * >( input->imageData );

    importFilter->SetImportPointer( localBuffer, numberOfPixels,
      importImageFilterWillOwnTheBuffer );
  }

  importFilter->Update();

  ItkImage::Pointer output = importFilter->GetOutput();

  output->DisconnectPipeline();

  mitkImage = mitk::ImportItkImage( output );

  return mitkImage;
} 