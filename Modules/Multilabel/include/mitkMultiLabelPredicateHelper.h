/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelPredicateHelper_h
#define mitkMultiLabelPredicateHelper_h

#include <MitkMultilabelExports.h>
#include <mitkNodePredicateBase.h>
#include <mitkBaseGeometry.h>

namespace mitk
{
  class DataStorage;

  /**
   * \brief Creates a predicate that matches valid multi-label segmentation data nodes.
   *
   * The predicate checks for:
   * - Data type is MultiLabelSegmentation
   * - The node is NOT a binary image
   * - The node is NOT a helper object
   * - Optionally: the node geometry is a sub-geometry of the given reference geometry
   *
   * \param[in] referenceGeometry Optional reference geometry for sub-geometry filtering.
   *            If nullptr, no geometry check is performed.
   * \return A NodePredicateBase smart pointer for filtering data nodes.
   * \sa GetSegmentationReferenceImagePredicate
   */
  mitk::NodePredicateBase::Pointer MITKMULTILABEL_EXPORT GetMultiLabelSegmentationPredicate(const mitk::BaseGeometry* referenceGeometry = nullptr);

  /**
   * \brief Creates a predicate that matches images suitable as reference for a segmentation.
   * \return A NodePredicateBase smart pointer for filtering reference image nodes.
   * \sa GetMultiLabelSegmentationPredicate
   */
  mitk::NodePredicateBase::Pointer MITKMULTILABEL_EXPORT GetSegmentationReferenceImagePredicate();

  /**
   * \brief Number of segmentation nodes whose geometry does not fit the reference.
   *
   * Counts data nodes that are multi-label segmentations (as by
   * GetMultiLabelSegmentationPredicate()) but whose geometry is not a sub-geometry
   * of referenceGeometry, i.e. exactly the segmentations that
   * GetMultiLabelSegmentationPredicate(referenceGeometry) excludes solely because
   * of the geometry restriction. This is deliberately geometry-specific so the
   * result cannot be conflated with segmentations filtered for other reasons.
   *
   * \param[in] dataStorage The data storage to inspect.
   * \param[in] referenceGeometry The geometry segmentations are expected to fit.
   * \return The number of geometry-mismatched segmentations. Returns 0 if
   *         dataStorage or referenceGeometry is null.
   * \sa GetMultiLabelSegmentationPredicate
   */
  unsigned int MITKMULTILABEL_EXPORT GetGeometryMismatchedSegmentationCount(const mitk::DataStorage* dataStorage, const mitk::BaseGeometry* referenceGeometry);
}

#endif
