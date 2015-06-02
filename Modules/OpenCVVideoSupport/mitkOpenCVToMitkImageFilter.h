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

// mitk includes
#include <MitkOpenCVVideoSupportExports.h>
#include <mitkCommon.h>
#include <mitkImageSource.h>

// itk includes
#include <itkMacro.h>
#include <itkImage.h>

// OpenCV includes
#include <cv.h>

namespace mitk
{

///
/// \brief Filter for creating MITK RGB Images from an OpenCV image
///
class MITKOPENCVVIDEOSUPPORT_EXPORT OpenCVToMitkImageFilter : public ImageSource
{
  public:
    typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
    typedef itk::RGBPixel< unsigned short > USRGBPixelType;
    typedef itk::RGBPixel< float > FloatRGBPixelType;
    typedef itk::RGBPixel< double > DoubleRGBPixelType;

    ///
    /// the static function for the conversion
    ///
    template <typename TPixel, unsigned int VImageDimension>
    static Image::Pointer ConvertIplToMitkImage( const IplImage * input );

    mitkClassMacro(OpenCVToMitkImageFilter, ImageSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

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

    OutputImageType* GetOutput(void);

    //##Documentation
    //## @brief Convenient method to insert an openCV image as a slice at a
    //## certain time step into a 3D or 4D mitk::Image.
    //##
    //## @param openCVImage - the image that is inserted into the mitk Image
    //## @param mitkImage - pointer to the mitkImage, which is changed by this method!
    //## @param timeStep - the time step, at which the openCVImage is inserted
    //##
    //## @attention The parameter mitkImage will be changed!
    void InsertOpenCVImageAsMitkTimeSlice(const cv::Mat openCVImage, Image::Pointer mitkImage, int timeStep);

  protected:

    OpenCVToMitkImageFilter(); // purposely hidden
    virtual ~OpenCVToMitkImageFilter();

    virtual void GenerateData() override;

protected:
    Image::Pointer m_Image;
    const IplImage* m_OpenCVImage;
    cv::Mat m_OpenCVMat;
};

} // namespace mitk

#endif // mitkOpenCVToMitkImageFilter_h
