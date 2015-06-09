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
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
    * \brief Converts given image to grayscale.
    * \return always true
    */
  bool OnFilterImage( cv::Mat& image ) override;
};

} // namespace mitk

#endif // mitkConvertGrayscaleOpenCVImageFilter_h
