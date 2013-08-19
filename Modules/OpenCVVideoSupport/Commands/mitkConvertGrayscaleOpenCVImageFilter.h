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

class MITK_OPENCVVIDEOSUPPORT_EXPORT ConvertGrayscaleOpenCVImageFilter : public AbstractOpenCVImageFilter
{

public:
  mitkClassMacro(ConvertGrayscaleOpenCVImageFilter, AbstractOpenCVImageFilter);
  itkNewMacro(Self);

  /**
    * \brief Converts given image to grayscale.
    * \return always true
    */
  bool FilterImage( cv::Mat& image );
};

} // namespace mitk

#endif // mitkConvertGrayscaleOpenCVImageFilter_h
