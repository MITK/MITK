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

#ifndef mitkCropOpenCVImageFilter_h
#define mitkCropOpenCVImageFilter_h

#include "mitkOpenCVVideoSupportExports.h"

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
class MITK_OPENCVVIDEOSUPPORT_EXPORT AbstractOpenCVImageFilter : public itk::Object
{

public:
  mitkClassMacro(AbstractOpenCVImageFilter, itk::Object);

  /**
    * \brief Pure virtual method for filtering an image.
    *
    * \param image OpenCV image which is supposed to be manipulated.
    * \return true if filtering was successfull, false otherwise
    */
  virtual bool FilterImage( cv::Mat& image ) = 0;
};

} // namespace mitk

#endif // mitkCropOpenCVImageFilter_h
