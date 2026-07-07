/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageStatisticsPredicateHelper_h
#define mitkImageStatisticsPredicateHelper_h

#include <MitkImageStatisticsExports.h>
#include <mitkNodePredicateBase.h>

namespace mitk
{
  /**
   * \brief Get a predicate for the ImageStatistics plugin input image.
   *
   * The predicate matches: DataType:Image && !(Property:binary) && !(Property:helper object).
   *
   * \return A node predicate suitable for selecting input images.
   */
  mitk::NodePredicateBase::Pointer MITKIMAGESTATISTICS_EXPORT GetImageStatisticsImagePredicate();

  /**
   * \brief Get a predicate for the ImageStatistics plugin input mask.
   *
   * The predicate matches multi-label segmentations; see GetMultiLabelSegmentationPredicate().
   *
   * \return A node predicate suitable for selecting mask images.
   */
  mitk::NodePredicateBase::Pointer MITKIMAGESTATISTICS_EXPORT GetImageStatisticsMaskPredicate();

  /**
   * \brief Get a predicate for the ImageStatistics plugin planar figure.
   *
   * The predicate matches: (DataType:PlanarCircle || DataType:PlanarRectangle ||
   * DataType:PlanarEllipse || DataType:PlanarDoubleEllipse || DataType:PlanarPolygon ||
   * DataType:PlanarSubdivisionPolygon || DataType:PlanarBezierCurve || DataType:PlanarLine)
   * && !(Property:helper object).
   *
   * \return A node predicate suitable for selecting planar figures.
   */
  mitk::NodePredicateBase::Pointer MITKIMAGESTATISTICS_EXPORT GetImageStatisticsPlanarFigurePredicate();
}

#endif
