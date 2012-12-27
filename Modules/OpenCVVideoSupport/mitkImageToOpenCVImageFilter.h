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

    int GetDepth(const std::type_info& typeInfo) const;

    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image );

    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing( itk::Image<itk::RGBPixel<TPixel>,VImageDimension>* image );

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
    const unsigned int numberOfComponents = pType.GetNumberOfComponents();
    const unsigned int numberOfPixels = m_Image->GetDimension(0) * m_Image->GetDimension(1);
    const unsigned int numberOfValues = numberOfPixels * numberOfComponents;

//    const unsigned int numberOfBytes = numberOfValues * sizeof( typename ImageType::PixelType );

    const typename ImageType::PixelType * itkBuffer = image->GetBufferPointer();
    typename ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
    // create new opencv image
    m_OpenCVImage = cvCreateImage( cvSize( size[0], size[1] )
      , GetDepth(typeid(TPixel)), numberOfComponents );
    const unsigned int stepsize = m_OpenCVImage->widthStep;
    const unsigned int width = m_OpenCVImage->width;
    const unsigned int height = m_OpenCVImage->height;
//    memcpy( m_OpenCVImage->imageData, itkBuffer, numberOfBytes );
    TPixel* mitkImagedata = (TPixel*)m_Image->GetData();
    TPixel* cvdata= reinterpret_cast<TPixel*>(m_OpenCVImage->imageData);
    for(int y = 0 ; y < height; y++)
    {
        for(int x = 0 ; x < width*numberOfComponents ; x+=numberOfComponents)
        {
            for(int c = 0 ; c < numberOfComponents ; c++)
            {
                cvdata[(numberOfComponents-c-1)+x] =mitkImagedata[x+c];
            }
        }
        cvdata = (TPixel*)(((unsigned char*)cvdata)+stepsize);
        mitkImagedata+= width*numberOfComponents;
    }
  }

  template<typename TPixel, unsigned int VImageDimension>
  void mitk::ImageToOpenCVImageFilter::ItkImageProcessing( itk::Image<itk::RGBPixel<TPixel>,VImageDimension>* image )
  {
    typedef itk::RGBPixel<TPixel> RGBPixelType;
    typedef itk::Image<RGBPixelType, VImageDimension> RGBImageType;
    typedef itk::ImageRegionIterator<RGBImageType> RGBIteratorType;

    RGBIteratorType it(image, image->GetLargestPossibleRegion());

    typename RGBImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
    // create new opencv image
    m_OpenCVImage = cvCreateImage( cvSize( size[0], size[1] ), GetDepth(typeid(RGBPixelType)), 3 );

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


