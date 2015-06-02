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

class MITKOPENCVVIDEOSUPPORT_EXPORT CropOpenCVImageFilter : public AbstractOpenCVImageFilter
{

public:
  mitkClassMacro(CropOpenCVImageFilter, AbstractOpenCVImageFilter);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  CropOpenCVImageFilter( );

  /**
    * \brief Crops image to rectangle given by mitk::CropOpenCVImageFilter::SetCropRegion.
    * \return false if no crop region was set or the crop region width is zero, true otherwise.
    */
  bool OnFilterImage( cv::Mat& image ) override;

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

  /**
    * \return True if a non-empty crop region was set before.
    */
  bool GetIsCropRegionEmpty( );

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
