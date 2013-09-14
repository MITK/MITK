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
#include <cv.h>

#include "mitkOpenCVVideoSupportExports.h"

namespace mitk
{

///
/// \brief Filter for creating MITK RGB Images from an OpenCV image
///
class MITK_OPENCVVIDEOSUPPORT_EXPORT OpenCVToMitkImageFilter : public ImageSource
{
  public:
    typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
    typedef itk::RGBPixel< unsigned short > USRGBPixelType;
    typedef itk::RGBPixel< float > FloatRGBPixelType;
    typedef itk::RGBPixel< double > DoubleRGBPixelType;

    ///
    /// the static function for the conversion
    /// WARNING: copyBuffer is deprecated, data will always be copied
    ///
    template <typename TPixel, unsigned int VImageDimension>
    static mitk::Image::Pointer ConvertIplToMitkImage( const IplImage * input, bool copyBuffer=true );

    mitkClassMacro(OpenCVToMitkImageFilter, ImageSource);
    itkNewMacro(OpenCVToMitkImageFilter);

    ///
    /// sets an iplimage as input
    ///
    void SetOpenCVImage(const IplImage* image);
    itkGetMacro(OpenCVImage, const IplImage*);

    ///
    /// sets an opencv mat as input (will be used if OpenCVImage Ipl image is 0)
    ///
    void SetOpenCVMat(const cv::Mat& image);
    itkGetMacro(OpenCVMat, cv::Mat);

    DEPRECATED( void SetCopyBuffer( bool ); );
    DEPRECATED( bool GetCopyBuffer(); );

    OutputImageType* GetOutput(void);

  protected:

    OpenCVToMitkImageFilter(); // purposely hidden
    virtual ~OpenCVToMitkImageFilter();

    virtual void GenerateData();

protected:
    mitk::Image::Pointer m_Image;
    const IplImage* m_OpenCVImage;
    cv::Mat m_OpenCVMat;
};

} // namespace

#endif // mitkOpenCVToMitkImageFilter_h
