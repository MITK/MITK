/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKNODEPREDICATES_H
#define MITKNODEPREDICATES_H

#include <MitkSemanticRelationsExports.h>

// mitk core
#include <mitkNodePredicateAnd.h>

namespace mitk
{
  namespace NodePredicates
  {
    /*
    * @brief Helper function to get a node predicate that can be used to filter images.
    *
    *        The images are of type 'mitk::Image' but must not be 'helper objects' or 'segmentation nodes'.
    *        For the definition of 'segmentation nodes' see 'GetSegmentationPredicate'.
    */
    MITKSEMANTICRELATIONS_EXPORT NodePredicateAnd::Pointer GetImagePredicate();
    /*
    * @brief Helper function to get a node predicate that can be used to filter segmentations.
    *
    *        The segmentations are of type 'mitk::LabelSetImage' or nodes that have their 'binary' property set to true.
    *        Segmentations must not be 'helper objects'.
    */
    MITKSEMANTICRELATIONS_EXPORT NodePredicateAnd::Pointer GetSegmentationPredicate();

  } // namespace NodePredicates
} // namespace mitk

#endif // MITKNODEPREDICATES_H
