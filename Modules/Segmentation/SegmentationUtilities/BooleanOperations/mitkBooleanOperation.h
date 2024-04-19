/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBooleanOperation_h
#define mitkBooleanOperation_h

#include <MitkSegmentationExports.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /** \brief Executes a boolean operation on two different segmentations.
   * All parameters of the boolean operations must be specified during construction.
   * The actual operation is executed when calling GetResult().
   */
  namespace BooleanOperation
  {

    Image::Pointer MITKSEGMENTATION_EXPORT GenerateUnion(const LabelSetImage*, LabelSetImage::LabelValueVectorType labelValues,
      std::function<void(float progress)> progressCallback = [](float) {});
    Image::Pointer MITKSEGMENTATION_EXPORT GenerateIntersection(const LabelSetImage*, LabelSetImage::LabelValueVectorType labelValues,
      std::function<void(float progress)> progressCallback = [](float) {});
    Image::Pointer MITKSEGMENTATION_EXPORT GenerateDifference(const LabelSetImage*, LabelSetImage::LabelValueType minuendLabelValue,
      const LabelSetImage::LabelValueVectorType subtrahendLabelValues, std::function<void(float progress)> progressCallback = [](float) {});

  };
}

#endif
