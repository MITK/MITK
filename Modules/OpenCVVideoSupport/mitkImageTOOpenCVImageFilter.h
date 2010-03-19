/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-28 18:32:03 +0100 (Do, 28 Jan 2010) $
Version:   $Revision: 21147 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkImageToOpenCVImageFilter_h
#define mitkImageToOpenCVImageFilter_h

#include <mitkCommon.h>
#include <mitkImage.h>
#include <mitkWeakPointer.h>
#include <itkMacro.h>
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkImageRegionIterator.h>
#include <cv.h>

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

    ImageToOpenCVImageFilter(); // purposely hidden
    virtual ~ImageToOpenCVImageFilter();

    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image, int depth );

    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing( itk::Image<itk::RGBPixel<TPixel>,VImageDimension>* image, int depth );

  protected:
    ///
    /// Saves if the filter should copy the data or just reference it
    ///
    mitk::WeakPointer<mitk::Image> m_Image;
    IplImage* m_OpenCVImage;
};

  template<typename TPixel, unsigned int VImageDimension>
  void mitk::ImageToOpenCVImageFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image, int depth )
  {
    typedef itk::Image<TPixel, VImageDimension> ImageType;

    const unsigned int numberOfPixels = m_OpenCVImage->width * m_OpenCVImage->height;
    const unsigned int numberOfBytes = numberOfPixels * sizeof( ImageType::PixelType );

    const ImageType::PixelType * itkBuffer = image->GetBufferPointer();

    ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
    // create new opencv image
    m_OpenCVImage = cvCreateImage( cvSize( size[0], size[1] )
      , depth, 1 );

    memcpy( m_OpenCVImage->imageData, itkBuffer, numberOfBytes );
  }

  template<typename TPixel, unsigned int VImageDimension>
  void mitk::ImageToOpenCVImageFilter::ItkImageProcessing( itk::Image<itk::RGBPixel<TPixel>,VImageDimension>* image, int depth )
  {
    typedef itk::RGBPixel<TPixel> RGBPixelType;
    typedef itk::Image<RGBPixelType, VImageDimension> RGBImageType;
    typedef itk::ImageRegionIterator<RGBImageType> RGBIteratorType;

    RGBIteratorType it(image, image->GetLargestPossibleRegion());

    RGBImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
    // create new opencv image
    m_OpenCVImage = cvCreateImage( cvSize( size[0], size[1] ), depth, 3 );

    unsigned int x = 0,y = 0;
    CvScalar s;
    for ( it.GoToBegin(); !it.IsAtEnd(); ++it ) 
    {
      s.val[0] = it.Value().GetBlue();
      s.val[1] = it.Value().GetGreen();
      s.val[2] = it.Value().GetRed();
      
      //MITK_DEBUG << "[" << x << "," << y << "] " << s.val[0] << "(B)," << s.val[1] << "(G)," << s.val[2] << "(R)";

      cvSet2D(m_OpenCVImage,y,x,s);

      ++x;
      // next line found
      if( x == size[0] )
      {
        x = 0;
        ++y;
      }
    }

  }
} // namespace

#endif // mitkImageToOpenCVImageFilter_h


