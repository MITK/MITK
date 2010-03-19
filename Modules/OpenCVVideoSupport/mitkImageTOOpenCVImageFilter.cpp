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

#include "mitkImageToOpenCVImageFilter.h"

#include <itkImportImageFilter.h>
#include <itkRGBPixel.h>
#include <mitkImageAccessByItk.h>

mitk::ImageToOpenCVImageFilter::ImageToOpenCVImageFilter()
: m_OpenCVImage(0)
{
}

mitk::ImageToOpenCVImageFilter::~ImageToOpenCVImageFilter()
{  
  m_OpenCVImage = 0;
}

bool mitk::ImageToOpenCVImageFilter::CheckImage( mitk::Image* image )
{
  if(image == 0)
  {
    MITK_WARN << "MITK Image is 0";
    return false;
  }
  if(image->GetDimension() != 2)
  {
    MITK_WARN << "Only 2D Images allowed";
    return false;
  }
  return true;
}

IplImage* mitk::ImageToOpenCVImageFilter::GetOpenCVImage()
{
  if(!m_OpenCVImage && this->CheckImage( m_Image ) == true )
  {
    // gray image
    if(m_Image->GetPixelType() == typeid(unsigned char))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, unsigned char, 2, IPL_DEPTH_8U );
    }
    else if(m_Image->GetPixelType() == typeid(char))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, char, 2, IPL_DEPTH_8S );
    }
    else if(m_Image->GetPixelType() == typeid(unsigned short))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, unsigned short, 2, IPL_DEPTH_16U );
    }
    else if(m_Image->GetPixelType() == typeid(short))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, short, 2, IPL_DEPTH_16S );
    }
    else if(m_Image->GetPixelType() == typeid(int))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, int, 2, IPL_DEPTH_32S );
    }
    else if(m_Image->GetPixelType() == typeid(float))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, float, 2, IPL_DEPTH_32F );
    }
    else if(m_Image->GetPixelType() == typeid(double))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, double, 2, IPL_DEPTH_64F );
    }

      // rgb image
    else if(m_Image->GetPixelType() == typeid(UCRGBPixelType))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, UCRGBPixelType, 2, IPL_DEPTH_8U  );
    }
    else if(m_Image->GetPixelType() == typeid(USRGBPixelType))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, USRGBPixelType, 2, IPL_DEPTH_16U );
    }
    else if(m_Image->GetPixelType() == typeid(FloatRGBPixelType))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, FloatRGBPixelType, 2, IPL_DEPTH_64F );
    }
    else if(m_Image->GetPixelType() == typeid(DoubleRGBPixelType))
    {
      AccessFixedTypeByItk_1 ( m_Image.GetPointer(), ItkImageProcessing, DoubleRGBPixelType, 2, IPL_DEPTH_64F );
    }

  }
  return m_OpenCVImage;
}

void mitk::ImageToOpenCVImageFilter::SetImage( mitk::Image* _Image )
{
  if(m_Image == _Image) return;
  m_Image = _Image;
  m_OpenCVImage = 0;
}