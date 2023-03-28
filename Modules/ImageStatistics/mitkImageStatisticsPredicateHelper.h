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
  @brief Gets a predicate for the ImageStatistics plugin input image
  @details Predicate: DataType:Image && !(Property:binary) && !(Property:helper object)
  */
  mitk::NodePredicateBase::Pointer MITKIMAGESTATISTICS_EXPORT GetImageStatisticsImagePredicate();
  /**
  @brief Gets a predicate for the ImageStatistics plugin input mask
  @details Predicate: ((DataType:Image && Property:binary) || DataType:LabelSetImage) && !(Property:helper object)
  */
  mitk::NodePredicateBase::Pointer MITKIMAGESTATISTICS_EXPORT GetImageStatisticsMaskPredicate();
  /**
  @brief Gets a predicate for the ImageStatistics plugin planar figure
  @details Predicate: (DataType:PlanarCircle || DataType:PlanarRectangle || DataType:PlanarEllipse
  DataType:PlanarDoubleEllipse || DataType:PlanarPolygon || DataType:PlanarSubdivisionPolygon ||
  DataType:PlanarBezierCurve || DataType:PlanarLine) && !(Property:helper object)
  */
  mitk::NodePredicateBase::Pointer MITKIMAGESTATISTICS_EXPORT GetImageStatisticsPlanarFigurePredicate();
}

#endif
