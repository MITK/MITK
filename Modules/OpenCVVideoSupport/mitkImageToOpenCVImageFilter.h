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

#include "mitkImageSliceSelector.h"

namespace mitk
{

///
/// \brief A pseudo-Filter for creating OpenCV images from MITK images with the option of copying data or referencing it
///
class MITKOPENCVVIDEOSUPPORT_EXPORT ImageToOpenCVImageFilter : public itk::Object
{
    public:
        typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
        typedef itk::RGBPixel< unsigned short > USRGBPixelType;
        typedef itk::RGBPixel< float > FloatRGBPixelType;
        typedef itk::RGBPixel< double > DoubleRGBPixelType;

        mitkClassMacroItkParent(ImageToOpenCVImageFilter, itk::Object);
        itkFactorylessNewMacro(Self)
        itkCloneMacro(Self)

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

        //##Documentation
        //## @brief Convenient method to set a certain slice of a 3D or 4D mitk::Image as input to convert it to an openCV image
        //##
        //## This methods sets the input. Call GetOpenCVMat() or GetOpenCVImage() to get the image.
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
        ~ImageToOpenCVImageFilter();

        ///
        /// Saves if the filter should copy the data or just reference it
        ///
        mitk::WeakPointer<mitk::Image> m_Image;
        IplImage* m_OpenCVImage;

  private:
    ImageSliceSelector::Pointer m_sliceSelector;
};

} // namespace

#endif // mitkImageToOpenCVImageFilter_h


