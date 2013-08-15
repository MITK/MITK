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

#include "mitkOpenCVToMitkImageFilter.h"

#include <itkImportImageFilter.h>
#include <itkRGBPixel.h>
#include <mitkITKImageImport.txx>
#include <itkOpenCVImageBridge.h>
#include <itkImageFileWriter.h>

mitk::OpenCVToMitkImageFilter::OpenCVToMitkImageFilter()
: m_OpenCVImage(0)
{
}

mitk::OpenCVToMitkImageFilter::~OpenCVToMitkImageFilter()
{
}

void mitk::OpenCVToMitkImageFilter::SetOpenCVImage(const IplImage* image)
{
  this->m_OpenCVImage = image;
  this->Modified();
}

void mitk::OpenCVToMitkImageFilter::GenerateData()
{
  IplImage cvMatIplImage;
  const IplImage* targetImage = 0;
  if(m_OpenCVImage == 0)
  {
      if( m_OpenCVMat.cols == 0 || m_OpenCVMat.rows == 0 )
      {
        MITK_WARN << "Cannot not start filter. OpenCV Image not set.";
        return;
      }
      else
      {
          cvMatIplImage = m_OpenCVMat;
          targetImage = &cvMatIplImage;
      }
  }
  else
      targetImage = m_OpenCVImage;

  // now convert rgb image
  if( (targetImage->depth>=0) && ((unsigned int)targetImage->depth == IPL_DEPTH_8S) && (targetImage->nChannels == 1) )
    m_Image = ConvertIplToMitkImage< char, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_8U && targetImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< unsigned char, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_8U && targetImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< UCRGBPixelType, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_16U && targetImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< unsigned short, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_16U && targetImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< USRGBPixelType, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_32F && targetImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< float, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_32F && targetImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< FloatRGBPixelType , 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_64F && targetImage->nChannels == 1 )
    m_Image = ConvertIplToMitkImage< double, 2>( targetImage );

  else if( targetImage->depth == IPL_DEPTH_64F && targetImage->nChannels == 3 )
    m_Image = ConvertIplToMitkImage< DoubleRGBPixelType , 2>( targetImage );

  else
  {
    MITK_WARN << "Unknown image depth and/or pixel type. Cannot convert OpenCV to MITK image.";
    return;
  }

  //cvReleaseImage(&rgbOpenCVImage);
}

mitk::ImageSource::OutputImageType* mitk::OpenCVToMitkImageFilter::GetOutput()
{
  return m_Image;
}

/********************************************
* Converting from OpenCV image to ITK Image
*********************************************/
template <typename TPixel, unsigned int VImageDimension>
mitk::Image::Pointer mitk::OpenCVToMitkImageFilter::ConvertIplToMitkImage( const IplImage * input,  bool )
{
  mitk::Image::Pointer mitkImage(0);

  typedef itk::Image< TPixel, VImageDimension > ImageType;

  typename ImageType::Pointer output = itk::OpenCVImageBridge::IplImageToITKImage<ImageType>(input);

  mitkImage = mitk::GrabItkImageMemory(output);

  return mitkImage;
}


void mitk::OpenCVToMitkImageFilter::SetOpenCVMat(const cv::Mat &image)
{
    m_OpenCVMat = image;
}


void mitk::OpenCVToMitkImageFilter::SetCopyBuffer(bool)
{
}

bool mitk::OpenCVToMitkImageFilter::GetCopyBuffer()
{
    return true;
}
