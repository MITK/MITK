/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelPredicateHelper.h"

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkLabelSetImage.h>

mitk::NodePredicateBase::Pointer mitk::GetMultiLabelSegmentationPredicate(const mitk::BaseGeometry* referenceGeometry)
{
  auto segPredicate = mitk::NodePredicateAnd::New();
  segPredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  segPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  segPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  if (nullptr != referenceGeometry)
    segPredicate = mitk::NodePredicateAnd::New(mitk::NodePredicateSubGeometry::New(referenceGeometry),
      segPredicate.GetPointer()).GetPointer();

  return segPredicate.GetPointer();
}

mitk::NodePredicateBase::Pointer mitk::GetSegmentationReferenceImagePredicate()
{
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  auto isDti = mitk::NodePredicateDataType::New("TensorImage");
  auto isOdf = mitk::NodePredicateDataType::New("OdfImage");
  auto isSegment = mitk::NodePredicateDataType::New("Segment");

  auto validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateNot::New(isSegment)));
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isOdf);

  auto referencePredicate = mitk::NodePredicateAnd::New();
  referencePredicate->AddPredicate(validImages);
  referencePredicate->AddPredicate(mitk::NodePredicateNot::New(GetMultiLabelSegmentationPredicate()));
  referencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  referencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  return referencePredicate.GetPointer();
}


