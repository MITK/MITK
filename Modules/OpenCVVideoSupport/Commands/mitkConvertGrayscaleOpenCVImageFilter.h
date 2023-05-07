/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConvertGrayscaleOpenCVImageFilter_h
#define mitkConvertGrayscaleOpenCVImageFilter_h

#include "mitkAbstractOpenCVImageFilter.h"

//itk headers
#include "itkObjectFactory.h"

namespace mitk {

class MITKOPENCVVIDEOSUPPORT_EXPORT ConvertGrayscaleOpenCVImageFilter : public AbstractOpenCVImageFilter
{

public:
  mitkClassMacro(ConvertGrayscaleOpenCVImageFilter, AbstractOpenCVImageFilter);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  /**
    * \brief Converts given image to grayscale.
    * \return always true
    */
  bool OnFilterImage( cv::Mat& image ) override;
};

} // namespace mitk

#endif
