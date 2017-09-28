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

#include "mitkNodePredicates.h"

// mitk core
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>

// multi label module
#include <mitkLabelSetImage.h>

mitk::NodePredicateAnd::Pointer mitk::NodePredicates::GetImagePredicate()
{
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(isImage);

  mitk::NodePredicateAnd::Pointer imagePredicate = mitk::NodePredicateAnd::New();
  imagePredicate->AddPredicate(validImages);
  imagePredicate->AddPredicate(mitk::NodePredicateNot::New(GetSegmentationPredicate()));
  imagePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  return imagePredicate;
}

mitk::NodePredicateAnd::Pointer mitk::NodePredicates::GetSegmentationPredicate()
{
  mitk::NodePredicateAnd::Pointer segmentationPredicate = mitk::NodePredicateAnd::New();

  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::TNodePredicateDataType<mitk::LabelSetImage>::Pointer isLabelSetImage = mitk::TNodePredicateDataType<mitk::LabelSetImage>::New();
  mitk::NodePredicateOr::Pointer allSegmentations = mitk::NodePredicateOr::New(isBinary, isLabelSetImage);

  segmentationPredicate->AddPredicate(allSegmentations);
  segmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  return segmentationPredicate;
}
