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

#ifndef mitkOpenCVToMitkImageFilter_h
#define mitkOpenCVToMitkImageFilter_h

#include <mitkCommon.h>
#include <mitkImageSource.h>
#include <itkMacro.h>
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <cv.h>

#include "mitkOpenCVVideoSupportExports.h"

namespace mitk
{

/**
  \brief Filter for creating MITK RGB Images from an OpenCV image

  Last contributor: $Author: mueller $
*/
class MITK_OPENCVVIDEOSUPPORT_EXPORT OpenCVToMitkImageFilter : public ImageSource
{
  public:
    typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
    typedef itk::RGBPixel< unsigned short > USRGBPixelType;
    typedef itk::RGBPixel< float > FloatRGBPixelType;
    typedef itk::RGBPixel< double > DoubleRGBPixelType;

    template <typename TPixel, unsigned int VImageDimension>
    static mitk::Image::Pointer ConvertIplToMitkImage( const IplImage * input, bool copyBuffer = true );

    mitkClassMacro(OpenCVToMitkImageFilter, ImageSource);
    itkNewMacro(OpenCVToMitkImageFilter);

    void SetOpenCVImage(const IplImage* image);
    itkGetMacro(OpenCVImage, const IplImage*);

    itkSetMacro(CopyBuffer, bool);
    itkGetMacro(CopyBuffer, bool);

    virtual DataObjectPointer MakeOutput(unsigned int idx);
    OutputImageType* GetOutput(unsigned int idx);

  protected:

    OpenCVToMitkImageFilter(); // purposely hidden
    virtual ~OpenCVToMitkImageFilter();

    virtual void GenerateData();

protected:
    mitk::Image::Pointer m_Image;
    const IplImage* m_OpenCVImage;
    bool m_CopyBuffer;
};

} // namespace

#endif // mitkOpenCVToMitkImageFilter_h


