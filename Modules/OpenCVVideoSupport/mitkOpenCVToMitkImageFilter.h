/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOpenCVToMitkImageFilter_h
#define mitkOpenCVToMitkImageFilter_h

// mitk includes
#include <MitkOpenCVVideoSupportExports.h>
#include <mitkCommon.h>
#include <mitkImageSource.h>

// itk includes
#include <itkMacro.h>
#include <itkFastMutexLock.h>
#include <itkImage.h>

// OpenCV includes
#include <opencv2/core.hpp>

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
    static Image::Pointer ConvertCVMatToMitkImage(const cv::Mat input);

    mitkClassMacro(OpenCVToMitkImageFilter, ImageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      ///
      /// sets an iplimage as input
      ///
      void SetOpenCVImage(const IplImage* image);
    //itkGetMacro(OpenCVImage, const IplImage*);

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
    ~OpenCVToMitkImageFilter() override;

    void GenerateData() override;

  protected:
    Image::Pointer m_Image;
    cv::Mat m_OpenCVMat;

    itk::FastMutexLock::Pointer m_ImageMutex;
    itk::FastMutexLock::Pointer m_OpenCVMatMutex;
  };

} // namespace mitk

#endif // mitkOpenCVToMitkImageFilter_h
