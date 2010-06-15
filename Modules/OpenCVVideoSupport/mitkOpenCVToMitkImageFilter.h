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

    itkSetObjectMacro(OpenCVImage, const IplImage);
    itkGetMacro(OpenCVImage, const IplImage*);

    virtual DataObjectPointer MakeOutput(unsigned int idx);
    OutputImageType* GetOutput(unsigned int idx);

  protected:

    OpenCVToMitkImageFilter(); // purposely hidden
    virtual ~OpenCVToMitkImageFilter();

    virtual void GenerateData();

protected:
    mitk::Image::Pointer m_Image;
    const IplImage* m_OpenCVImage;
};

} // namespace

#endif // mitkOpenCVToMitkImageFilter_h


