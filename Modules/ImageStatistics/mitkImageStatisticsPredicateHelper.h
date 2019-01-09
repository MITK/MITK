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

#ifndef MITKPREDICATEHELPER_H
#define MITKPREDICATEHELPER_H

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
