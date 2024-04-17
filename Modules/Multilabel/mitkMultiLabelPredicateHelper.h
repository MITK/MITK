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
  /**
  @brief Gets a predicate for a valid multi label segmentation node
  @details Predicate: DataType:LabelSetImage && !(Property:binary) && !(Property:helper object) && isSubGeometry(referenceGeometry; if set) 
  */
  mitk::NodePredicateBase::Pointer MITKMULTILABEL_EXPORT GetMultiLabelSegmentationPredicate(const mitk::BaseGeometry* referenceGeometry = nullptr);
  /**
  @brief Gets a predicate for the images that can be used as reference for a segmentation.
  */
  mitk::NodePredicateBase::Pointer MITKMULTILABEL_EXPORT GetSegmentationReferenceImagePredicate();
}

#endif
