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

#ifndef mitkAbstractOpenCVImageFilter_h
#define mitkAbstractOpenCVImageFilter_h

#include "mitkAbstractOpenCVImageFilter.h"

#include "cv.h"

//itk headers
#include <itkObjectFactory.h>

namespace mitk {

class MITK_OPENCVVIDEOSUPPORT_EXPORT CropOpenCVImageFilter : public AbstractOpenCVImageFilter
{

public:
  mitkClassMacro(CropOpenCVImageFilter, AbstractOpenCVImageFilter);
  itkNewMacro(Self);

  /**
    * \brief Crops image to rectangle given by mitk::CropOpenCVImageFilter::SetCropRegion.
    * \return false if no crop region was set or the crop region width is zero, true otherwise.
    */
  bool filterImage( cv::Mat image );

  void SetCropRegion( cv::Rect cropRegion );
  cv::Rect GetCropRegion( );

protected:
  /**
  * \brief Defines the region which will be cropped from the image.
  */
  cv::Rect m_CropRegion;
};

} // namespace mitk

#endif // mitkAbstractOpenCVImageFilter_h
