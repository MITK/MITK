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
    : m_OpenCVImage(nullptr)
  {
    m_sliceSelector = ImageSliceSelector::New();
  }

  ImageToOpenCVImageFilter::~ImageToOpenCVImageFilter()
  {
    m_OpenCVImage = nullptr;
  }


  void ImageToOpenCVImageFilter::SetImage( Image* _Image )
  {
    m_Image = _Image;
  }


  bool ImageToOpenCVImageFilter::CheckImage( Image* image )
  {
    if(image == nullptr)
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
      return nullptr;

    m_OpenCVImage = (nullptr);

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
      return nullptr;
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

  void ImageToOpenCVImageFilter::SetInputFromTimeSlice(Image::Pointer mitkImage, int timeStep, int slice)
  {
    m_sliceSelector->SetInput(mitkImage);
    m_sliceSelector->SetSliceNr(slice);
    m_sliceSelector->SetTimeNr(timeStep);
    m_sliceSelector->Update();
    this->SetImage(m_sliceSelector->GetOutput());
  }

} // end namespace mitk