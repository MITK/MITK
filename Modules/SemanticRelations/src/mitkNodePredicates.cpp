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
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

// multi label module
#include <mitkLabelSetImage.h>

mitk::NodePredicateAnd::Pointer mitk::NodePredicates::GetImagePredicate()
{
  TNodePredicateDataType<Image>::Pointer isImage = TNodePredicateDataType<Image>::New();

  NodePredicateOr::Pointer validImages = NodePredicateOr::New();
  validImages->AddPredicate(isImage);

  NodePredicateAnd::Pointer imagePredicate = NodePredicateAnd::New();
  imagePredicate->AddPredicate(validImages);
  imagePredicate->AddPredicate(NodePredicateNot::New(GetSegmentationPredicate()));
  imagePredicate->AddPredicate(NodePredicateNot::New(NodePredicateProperty::New("helper object")));

  return imagePredicate;
}

mitk::NodePredicateAnd::Pointer mitk::NodePredicates::GetSegmentationPredicate()
{
  NodePredicateAnd::Pointer segmentationPredicate = NodePredicateAnd::New();

  NodePredicateProperty::Pointer isBinary = NodePredicateProperty::New("binary", BoolProperty::New(true));
  TNodePredicateDataType<LabelSetImage>::Pointer isLabelSetImage = TNodePredicateDataType<LabelSetImage>::New();
  NodePredicateOr::Pointer allSegmentations = NodePredicateOr::New(isBinary, isLabelSetImage);

  segmentationPredicate->AddPredicate(allSegmentations);
  segmentationPredicate->AddPredicate(NodePredicateNot::New(NodePredicateProperty::New("helper object")));

  return segmentationPredicate;
}
