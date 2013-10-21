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

namespace mitk{

  ImageToOpenCVImageFilter::ImageToOpenCVImageFilter()
    : m_OpenCVImage(0)
  {
  }

  ImageToOpenCVImageFilter::~ImageToOpenCVImageFilter()
  {
    m_OpenCVImage = 0;
  }


  void ImageToOpenCVImageFilter::SetImage( Image* _Image )
  {
    m_Image = _Image;
  }


  bool ImageToOpenCVImageFilter::CheckImage( Image* image )
  {
    if(image == 0)
    {
      MITK_WARN << "MITK Image is 0";
      return false;
    }
    if(image->GetDimension() > 2 )
    {
      MITK_WARN << "Only 2D Images allowed";
      return false;
    }
    return true;
  }

  IplImage* ImageToOpenCVImageFilter::GetOpenCVImage()
  {

    if(!this->CheckImage( m_Image ))
      return 0;

    m_OpenCVImage = (0);

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

  cv::Mat ImageToOpenCVImageFilter::GetOpenCVMat()
  {
    IplImage* img = this->GetOpenCVImage();

    cv::Mat mat;
    if( img )
    {
      // do not copy data, then release just the header
      mat = cv::Mat ( img, false );
      cvReleaseImageHeader( &img );
    }

    return mat;
  }

  template<typename TPixel, unsigned int VImageDimension>
  void ImageToOpenCVImageFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image )
  {
    m_OpenCVImage = itk::OpenCVImageBridge::ITKImageToIplImage(image);
  }

} // end namespace mitk