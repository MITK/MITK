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

#include "mitkImageStatisticsPredicateHelper.h"

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>

namespace mitk
{

  mitk::NodePredicateBase::Pointer GetNoHelperObjectPredicate()
  {
    auto hasHelperObjectProperty = mitk::NodePredicateProperty::New("helper object", nullptr);
    auto isNoHelperObject = mitk::NodePredicateNot::New(hasHelperObjectProperty);
    return isNoHelperObject.GetPointer();
  }

  mitk::NodePredicateBase::Pointer GetImageStatisticsImagePredicate()
  {
    auto isImage = mitk::NodePredicateDataType::New("Image");
    auto hasBinaryProperty = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    auto isNotBinary = mitk::NodePredicateNot::New(hasBinaryProperty);
    auto isNotBinaryImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
    auto isNoHelperObjectPredicate = GetNoHelperObjectPredicate();

    auto isImageForImageStatistics = mitk::NodePredicateAnd::New(isNotBinaryImage, isNoHelperObjectPredicate);
    return isImageForImageStatistics.GetPointer();
  }

  mitk::NodePredicateBase::Pointer GetImageStatisticsMaskPredicate()
  {
    auto isImage = mitk::NodePredicateDataType::New("Image");
    auto isLabelSetImage = mitk::NodePredicateDataType::New("LabelSetImage");
    auto hasBinaryProperty = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    auto isBinaryImage = mitk::NodePredicateAnd::New(isImage, hasBinaryProperty);
    auto isNoHelperObjectPredicate = GetNoHelperObjectPredicate();

    auto isLabelSetOrBinaryImage = mitk::NodePredicateOr::New(isLabelSetImage, isBinaryImage);
    auto isMaskForImageStatistics = mitk::NodePredicateAnd::New(isLabelSetOrBinaryImage, isNoHelperObjectPredicate);
    return isMaskForImageStatistics.GetPointer();
  }

  mitk::NodePredicateBase::Pointer GetImageStatisticsPlanarFigurePredicate()
  {
    auto isPlanarCircle = mitk::NodePredicateDataType::New("PlanarCircle");
    auto isPlanarRectangle = mitk::NodePredicateDataType::New("PlanarRectangle");
    auto isPlanarEllipse = mitk::NodePredicateDataType::New("PlanarEllipse");
    auto isPlanarDoubleEllipse = mitk::NodePredicateDataType::New("PlanarDoubleEllipse");
    auto isPlanarPolygon = mitk::NodePredicateDataType::New("PlanarPolygon");
    auto isPlanarSubdivisionPolygon = mitk::NodePredicateDataType::New("PlanarSubdivisionPolygon");
    auto isPlanarBezierCurve = mitk::NodePredicateDataType::New("PlanarBezierCurve");
    auto isPlanarLine = mitk::NodePredicateDataType::New("PlanarLine");
    auto isNoHelperObjectPredicate = GetNoHelperObjectPredicate();

    auto isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarCircle, isPlanarEllipse);
    isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarFigureForImageStatistics, isPlanarRectangle);
    isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarFigureForImageStatistics, isPlanarDoubleEllipse);
    isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarFigureForImageStatistics, isPlanarPolygon);
    isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarFigureForImageStatistics, isPlanarSubdivisionPolygon);
    isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarFigureForImageStatistics, isPlanarBezierCurve);
    isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New(isPlanarFigureForImageStatistics, isPlanarLine);
    auto isPlanarFigureAndNoHelperForImageStatistics = mitk::NodePredicateAnd::New(isPlanarFigureForImageStatistics, isNoHelperObjectPredicate);

    return isPlanarFigureAndNoHelperForImageStatistics.GetPointer();
  }
}
