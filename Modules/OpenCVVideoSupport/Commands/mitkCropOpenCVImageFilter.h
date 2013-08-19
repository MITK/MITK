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

  CropOpenCVImageFilter( );

  /**
    * \brief Crops image to rectangle given by mitk::CropOpenCVImageFilter::SetCropRegion.
    * \return false if no crop region was set or the crop region width is zero, true otherwise.
    */
  bool FilterImage( cv::Mat& image );

  /**
    * \brief Set region of interest for cropping.
    */
  void SetCropRegion( cv::Rect cropRegion );

  /**
    * \brief Set region of interest for cropping.
    */
  void SetCropRegion( int topLeftX, int topLeftY, int bottomRightX, int bottomRightY );

  /**
    * \brief Returns region, which was set by mitk::CropOpenCVImageFilter::SetCropRegion().
    */
  cv::Rect GetCropRegion( );

protected:
  /**
    * \brief Defines the region which will be cropped from the image.
    */
  cv::Rect m_CropRegion;

  /**
    * \brief True if no image was filtered since last set of a crop region.
    */
  bool m_NewCropRegionSet;
};

} // namespace mitk

#endif // mitkAbstractOpenCVImageFilter_h
