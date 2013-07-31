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

#ifndef mitkImageToOpenCVImageFilter_h
#define mitkImageToOpenCVImageFilter_h

#include <mitkCommon.h>
#include <mitkImage.h>
#include <mitkWeakPointer.h>
#include <itkMacro.h>
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <mitkPixelType.h>
#include <itkImageRegionIterator.h>
#include <cv.h>
#include <string>
#include <sstream>
#include "itkOpenCVImageBridge.h"

#include "mitkOpenCVVideoSupportExports.h"

namespace mitk
{

/**
  \brief A pseudo-Filter for creating OpenCV images from MITK images with the option of copying data or referencing it

  Last contributor: $Author: mueller $
*/
  class MITK_OPENCVVIDEOSUPPORT_EXPORT ImageToOpenCVImageFilter : public itk::Object
{
  public:
    typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
    typedef itk::RGBPixel< unsigned short > USRGBPixelType;
    typedef itk::RGBPixel< float > FloatRGBPixelType;
    typedef itk::RGBPixel< double > DoubleRGBPixelType;

    template <typename TPixel, unsigned int VImageDimension>
    static mitk::Image::Pointer ConvertIplToMitkImage( const IplImage * input, bool copyBuffer = true );

    mitkClassMacro(ImageToOpenCVImageFilter, itk::Object);
    itkNewMacro(ImageToOpenCVImageFilter);

    void SetImage( mitk::Image* _Image );
    itkGetMacro(Image, mitk::Image*);

    bool CheckImage(mitk::Image* image);
    ///
    /// Get the produced OpenCVImage.
    /// ATTENTION: Do not forget to release this image again with cvReleaseImage().
    ///
    IplImage* GetOpenCVImage();

  protected:
    ///
    /// Saves if the filter should copy the data or just reference it
    ///
    mitk::WeakPointer<mitk::Image> m_Image;
    IplImage* m_OpenCVImage;
};

  template<typename TPixel, unsigned int VImageDimension>
  void mitk::ImageToOpenCVImageFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image )
  {
    PixelType pType = m_Image->GetPixelType(0);
    typedef itk::Image<TPixel, VImageDimension> ImageType;

    // create new opencv image
    m_OpenCVImage = itk::OpenCVImageBridge::ITKImageToIplImage (image);
  }

} // namespace

#endif // mitkImageToOpenCVImageFilter_h


