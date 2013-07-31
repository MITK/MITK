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
  if(!this->CheckImage( m_Image ))
    return 0;


    try
    {
      AccessFixedTypeByItk(m_Image.GetPointer(), ItkImageProcessing,
                           MITK_ACCESSBYITK_PIXEL_TYPES_SEQ  // gray image
                           (UCRGBPixelType)(USRGBPixelType)(FloatRGBPixelType)(DoubleRGBPixelType), // rgb image
                           (2) // dimensions
                           )
    }
    catch (const AccessByItkException& e) {
      std::cout << "Caught exception [from AccessFixedTypeByItk]: \n" << e.what() << "\n";
      return 0;
    }

  return m_OpenCVImage;
}

void mitk::ImageToOpenCVImageFilter::SetImage( mitk::Image* _Image )
{
//  if(m_Image == _Image) return;
  m_Image = _Image;
}
