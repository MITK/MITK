/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCropOpenCVImageFilter_h
#define mitkCropOpenCVImageFilter_h

#include <MitkOpenCVVideoSupportExports.h>

#include <mitkCommon.h>

//itk headers
#include <itkMacro.h>

namespace cv {
class Mat;
}

namespace mitk {

/**
  * \brief Interface for image filters on OpenCV images.
  *
  * Every concrete filter has to implement the pure virual
  * mitk::AbstractOpenCVImageFilter::filterImage() method.
  *
  */
class MITKOPENCVVIDEOSUPPORT_EXPORT AbstractOpenCVImageFilter : public itk::Object
{

public:
  static int INVALID_IMAGE_ID;

  mitkClassMacroItkParent(AbstractOpenCVImageFilter, itk::Object);

  virtual bool FilterImage( cv::Mat& image, int id = INVALID_IMAGE_ID );

  /**
    * \brief Pure virtual method for filtering an image.
    *
    * \param image OpenCV image which is supposed to be manipulated.
    * \return true if filtering was successfull, false otherwise
    */
  virtual bool OnFilterImage( cv::Mat& image ) = 0;

protected:
  AbstractOpenCVImageFilter();

  int GetCurrentImageId();

  int m_CurrentImageId;
};

} // namespace mitk

#endif // mitkCropOpenCVImageFilter_h
