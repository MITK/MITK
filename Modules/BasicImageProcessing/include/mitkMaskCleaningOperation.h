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

#ifndef mitkMaskClearningOperation_h
#define mitkMaskClearningOperation_h

#include <mitkImage.h>
#include <MitkBasicImageProcessingExports.h>

namespace mitk
{

  /** \brief Executes operations to clean-up Masks.
  *
  * It is assumed that the segmentation is of type <unsigned short>
  */
  class MITKBASICIMAGEPROCESSING_EXPORT MaskCleaningOperation {
  public:
    /** \brief Limits a Mask to a given Range
    *
    * Removes all elements of a mask that are not within the given range. If lowerLimitOn is true, all voxels that
    * cover voxels with an intensity below lowerLimit will be set to 0 in the resulting mask. Similar, all voxels
    * that cover voxels with an intensity above upperLimit will be set to 0 if upperLimitOn is true.
    *
    * <b>Parameter</b>
    * - mitk::Image::Pointer image : Grey-Scale image
    * - mitk::Image::Pointer mask  : Original mask
    * - optional: bool lowerLimitOn : If true, voxels below lowerLimit are not masked, default false
    * - optional: double lowerLimit : Lower Threshold limit, default 0
    * - optional: bool upperLimitOn : If true, voxels above upperLimit are not masked, default false
    * - optional: double upperLimit : Lower Threshold limit, default 1
    */
    static Image::Pointer RangeBasedMasking(Image::Pointer & image, Image::Pointer & mask, bool lowerLimitOn=false, double lowerLimit=0, bool upperLimitOn=false, double upperLimit=1);

    /** \brief Removes outlier from a mask
    *
    * Removes all elements of a mask that are not within the range \f$ [\mu - 3 \sigma , \mu + 3 \sigma\] \f$
    *
    * <b>Parameter</b>
    * - mitk::Image::Pointer image : Grey-Scale image
    * - mitk::Image::Pointer mask  : Original mask
    */
    static Image::Pointer MaskOutlierFiltering(Image::Pointer & image, Image::Pointer & mask);

  };


}
#endif // mitkMaskClearningOperation_h