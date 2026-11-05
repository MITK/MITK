/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageStatisticsPredicateHelper.h>

#include <mitkMultiLabelPredicateHelper.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPath.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarSubdivisionPolygon.h>

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
    auto hasBinaryProperty = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    auto isBinaryImage = mitk::NodePredicateAnd::New(isImage, hasBinaryProperty);
    auto isNoHelperObjectPredicate = GetNoHelperObjectPredicate();
    auto isBinaryMaskForImageStatistics = mitk::NodePredicateAnd::New(isBinaryImage, isNoHelperObjectPredicate);

    return mitk::NodePredicateOr::New(GetMultiLabelSegmentationPredicate(), isBinaryMaskForImageStatistics).GetPointer();
  }

  mitk::NodePredicateBase::Pointer GetImageStatisticsPlanarFigurePredicate()
  {
    // Regions, and lines along which an intensity profile is computed.
    auto isPlanarFigureForImageStatistics = mitk::NodePredicateOr::New();
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarCircle>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarEllipse>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarRectangle>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarDoubleEllipse>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarPolygon>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarPath>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarSubdivisionPolygon>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarBezierCurve>::New());
    isPlanarFigureForImageStatistics->AddPredicate(mitk::TNodePredicateDataType<mitk::PlanarLine>::New());

    auto isNoHelperObjectPredicate = GetNoHelperObjectPredicate();
    auto isPlanarFigureAndNoHelperForImageStatistics = mitk::NodePredicateAnd::New(isPlanarFigureForImageStatistics, isNoHelperObjectPredicate);

    return isPlanarFigureAndNoHelperForImageStatistics.GetPointer();
  }
}
