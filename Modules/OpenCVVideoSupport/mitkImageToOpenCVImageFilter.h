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
#include <itkOpenCVImageBridge.h>
#include <MitkOpenCVVideoSupportExports.h>

namespace mitk
{

///
/// \brief A pseudo-Filter for creating OpenCV images from MITK images with the option of copying data or referencing it
///
class MITK_OPENCVVIDEOSUPPORT_EXPORT ImageToOpenCVImageFilter : public itk::Object
{
    public:
        typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
        typedef itk::RGBPixel< unsigned short > USRGBPixelType;
        typedef itk::RGBPixel< float > FloatRGBPixelType;
        typedef itk::RGBPixel< double > DoubleRGBPixelType;

        mitkClassMacro(ImageToOpenCVImageFilter, itk::Object);
        itkNewMacro(ImageToOpenCVImageFilter);

        ///
        /// \brief set the input MITK image
        ///
        void SetImage( mitk::Image* _Image );
        ///
        /// \brief get the input MITK image
        ///
        itkGetMacro(Image, mitk::Image*);

        ///
        /// \brief get the input MITK image
        ///
        bool CheckImage(mitk::Image* image);

        ///
        /// RUNS the conversion and returns the produced OpenCVImage.
        /// !!!ATTENTION!!! Do not forget to release this image again with cvReleaseImage().
        /// \return the produced OpenCVImage or 0 if an error occured!
        ///
        IplImage* GetOpenCVImage();

        ///
        /// RUNS the conversion and returns the produced image as cv::Mat.
        /// \return the produced OpenCVImage or an empty image if an error occured
        ///
        cv::Mat GetOpenCVMat();

    protected:
        ///
        /// the actual templated conversion method
        ///
        template<typename TPixel, unsigned int VImageDimension>
        void ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image );

        ImageToOpenCVImageFilter();
        ~ImageToOpenCVImageFilter();

        ///
        /// Saves if the filter should copy the data or just reference it
        ///
        mitk::WeakPointer<mitk::Image> m_Image;
        IplImage* m_OpenCVImage;
};

} // namespace

#endif // mitkImageToOpenCVImageFilter_h


