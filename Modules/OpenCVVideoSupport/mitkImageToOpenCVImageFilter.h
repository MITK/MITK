/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToOpenCVImageFilter_h
#define mitkImageToOpenCVImageFilter_h

#include <mitkCommon.h>
#include <mitkImage.h>
#include <mitkWeakPointer.h>
#include <itkOpenCVImageBridge.h>
#include <MitkOpenCVVideoSupportExports.h>
#include <opencv2/core.hpp>

#include "mitkImageSliceSelector.h"

namespace mitk
{

///
/// \brief A pseudo-filter for creating OpenCV images from MITK images with the option of copying data or referencing it
///
class MITKOPENCVVIDEOSUPPORT_EXPORT ImageToOpenCVImageFilter : public itk::Object
{
    public:
        typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
        typedef itk::RGBPixel< unsigned short > USRGBPixelType;
        typedef itk::RGBPixel< float > FloatRGBPixelType;
        typedef itk::RGBPixel< double > DoubleRGBPixelType;

        mitkClassMacroItkParent(ImageToOpenCVImageFilter, itk::Object);
        itkFactorylessNewMacro(Self);
        itkCloneMacro(Self);

        ///
        /// \brief set the input MITK image
        ///
        void SetImage( mitk::Image* _Image );
        ///
        /// \brief get the input MITK image
        ///
        mitk::Image* GetImage();

        ///
        /// \brief get the input MITK image
        ///
        bool CheckImage(mitk::Image* image);

        ///
        /// RUNS the conversion and returns the produced image as cv::Mat.
        /// \return the produced OpenCVImage or an empty image if an error occured
        ///
        cv::Mat GetOpenCVMat();

        //##Documentation
        //## @brief Convenient method to set a certain slice of a 3D or 4D mitk::Image as input to convert it to an openCV image
        //##
        //## This methods sets the input. Call GetOpenCVMat() to get the image.
        //##
        //## @param mitkImage - the image that should be converted to an openCVImage
        //## @param timeStep - the time step, which is converted to openCV
        //## @param slice - the slice which is converted to openCV
        void SetInputFromTimeSlice(Image::Pointer mitkImage, int timeStep, int slice);

    protected:
        ///
        /// the actual templated conversion method
        ///
        template<typename TPixel, unsigned int VImageDimension>
        void ItkImageProcessing( itk::Image<TPixel,VImageDimension>* image );

        ImageToOpenCVImageFilter();
        ~ImageToOpenCVImageFilter() override;

        ///
        /// Saves if the filter should copy the data or just reference it
        ///
        mitk::WeakPointer<mitk::Image> m_Image;
        cv::Mat m_OpenCVImage;

    private:
        ImageSliceSelector::Pointer m_sliceSelector;
};

} // namespace

#endif // mitkImageToOpenCVImageFilter_h


