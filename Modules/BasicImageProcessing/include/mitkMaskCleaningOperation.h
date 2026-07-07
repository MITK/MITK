/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaskCleaningOperation_h
#define mitkMaskCleaningOperation_h

#include <mitkImage.h>
#include <MitkBasicImageProcessingExports.h>

namespace mitk
{

  /**
  \brief Provides static methods for cleaning and filtering segmentation masks.

  The MaskCleaningOperation class provides operations to refine binary segmentation masks
  based on the intensity values of a corresponding grey-scale image. Supported operations
  include range-based masking (removing mask voxels outside a specified intensity range)
  and statistical outlier filtering (removing mask voxels where intensities fall outside
  three standard deviations from the mean).

  It is assumed that the segmentation mask is of type "unsigned short".

  \sa ArithmeticOperation, TransformationOperation
  */
  class MITKBASICIMAGEPROCESSING_EXPORT MaskCleaningOperation {
  public:
    /**
    \brief Remove mask voxels where the corresponding image intensity is outside a given range.

    For each voxel in the mask, the corresponding intensity in the grey-scale image is checked
    against optional lower and upper thresholds. If the intensity falls outside the active
    threshold bounds, the mask voxel is set to 0.

    \param[in] image Grey-scale image providing the intensity values for threshold comparison.
    \param[in] mask Original binary mask (unsigned short) to be filtered.
    \param[in] lowerLimitOn If true, mask voxels covering intensities below lowerLimit are set to 0. Default: false.
    \param[in] lowerLimit The lower intensity threshold. Only used when lowerLimitOn is true. Default: 0.
    \param[in] upperLimitOn If true, mask voxels covering intensities above upperLimit are set to 0. Default: false.
    \param[in] upperLimit The upper intensity threshold. Only used when upperLimitOn is true. Default: 1.
    \return A new mask image with out-of-range voxels set to 0.
    */
    static Image::Pointer RangeBasedMasking(Image::Pointer & image, Image::Pointer & mask, bool lowerLimitOn=false, double lowerLimit=0, bool upperLimitOn=false, double upperLimit=1);

    /**
    \brief Remove mask voxels where image intensities are statistical outliers.

    Computes the mean (\f$\mu\f$) and standard deviation (\f$\sigma\f$) of the image
    intensities within the masked region, then removes all mask voxels where the corresponding
    image intensity falls outside the range \f$[\mu - 3\sigma, \mu + 3\sigma]\f$.

    Internally delegates to RangeBasedMasking() with the computed statistical bounds.

    \param[in] image Grey-scale image providing the intensity values for outlier detection.
    \param[in] mask Original binary mask (unsigned short) defining the region of interest.
    \return A new mask image with outlier voxels set to 0.
    \sa RangeBasedMasking
    */
    static Image::Pointer MaskOutlierFiltering(Image::Pointer & image, Image::Pointer & mask);

  };


}
#endif
