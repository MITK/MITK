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
  /**
   * \brief Namespace providing boolean set operations on multi-label segmentations.
   *
   * Provides functions to compute the union, intersection, or difference of
   * label regions within a MultiLabelSegmentation. Each function returns a new
   * binary image representing the result of the operation.
   *
   * \sa MultiLabelSegmentation
   */
  namespace BooleanOperation
  {

    /**
     * \brief Computes the union (logical OR) of the specified label regions.
     * \param[in] segmentation Pointer to the source multi-label segmentation.
     * \param[in] labelValues Vector of label values whose regions are combined.
     * \param[in] progressCallback Optional callback for progress reporting (0.0 to 1.0).
     * \return A new binary image representing the union of all specified labels.
     * \pre segmentation must point to a valid instance.
     */
    Image::Pointer MITKSEGMENTATION_EXPORT GenerateUnion(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueVectorType labelValues,
      std::function<void(float progress)> progressCallback = [](float) {});

    /**
     * \brief Computes the intersection (logical AND) of the specified label regions.
     * \param[in] segmentation Pointer to the source multi-label segmentation.
     * \param[in] labelValues Vector of label values whose regions are intersected.
     * \param[in] progressCallback Optional callback for progress reporting (0.0 to 1.0).
     * \return A new binary image representing the intersection of all specified labels.
     * \pre segmentation must point to a valid instance.
     */
    Image::Pointer MITKSEGMENTATION_EXPORT GenerateIntersection(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueVectorType labelValues,
      std::function<void(float progress)> progressCallback = [](float) {});

    /**
     * \brief Computes the difference (set subtraction) of label regions.
     *
     * Subtracts the union of the subtrahend labels from the minuend label region.
     *
     * \param[in] segmentation Pointer to the source multi-label segmentation.
     * \param[in] minuendLabelValue Label value of the region to subtract from.
     * \param[in] subtrahendLabelValues Label values of the regions to subtract.
     * \param[in] progressCallback Optional callback for progress reporting (0.0 to 1.0).
     * \return A new binary image representing the difference.
     * \pre segmentation must point to a valid instance.
     */
    Image::Pointer MITKSEGMENTATION_EXPORT GenerateDifference(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueType minuendLabelValue,
      const MultiLabelSegmentation::LabelValueVectorType subtrahendLabelValues, std::function<void(float progress)> progressCallback = [](float) {});

  };
}

#endif
