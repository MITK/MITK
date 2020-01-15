/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageToOpenCVImageFilter.h"

#include <itkImportImageFilter.h>
#include <itkRGBPixel.h>
#include <mitkImageAccessByItk.h>

namespace mitk{

  ImageToOpenCVImageFilter::ImageToOpenCVImageFilter()
  {
    m_sliceSelector = ImageSliceSelector::New();
  }

  ImageToOpenCVImageFilter::~ImageToOpenCVImageFilter()
  {
  }


  void ImageToOpenCVImageFilter::SetImage( Image* _Image )
  {
    m_Image = _Image;
  }

  Image* ImageToOpenCVImageFilter::GetImage()
  {
    return m_Image.Lock();
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

  cv::Mat ImageToOpenCVImageFilter::GetOpenCVMat()
  {
    auto image = m_Image.Lock();

    if(!this->CheckImage(image))
      return cv::Mat();

    try
    {
      AccessFixedTypeByItk(image.GetPointer(), ItkImageProcessing,
        MITK_ACCESSBYITK_PIXEL_TYPES_SEQ  // gray image
        (UCRGBPixelType)(USRGBPixelType)(FloatRGBPixelType)(DoubleRGBPixelType), // rgb image
        (2) // dimensions
        )
    }
    catch (const AccessByItkException& e) {
      std::cout << "Caught exception [from AccessFixedTypeByItk]: \n" << e.what() << "\n";
      return cv::Mat();
    }
    return m_OpenCVImage;
  }

  template<typename TPixel, unsigned int VImageDimension>
  void ImageToOpenCVImageFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image )
  {
    m_OpenCVImage = itk::OpenCVImageBridge::ITKImageToCVMat(image);
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
