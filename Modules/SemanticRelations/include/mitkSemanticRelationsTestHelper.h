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

#ifndef MITKSEMANTICRELATIONSTESTHELPER_H
#define MITKSEMANTICRELATIONSTESTHELPER_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkSemanticRelations.h"

namespace mitk
{
  namespace SemanticRelationsTestHelper
  {
    /************************************************************************/
    /* functions to add/remove image and segmentation instances				      */
    /************************************************************************/

    /*
    * @brief  Add a new image to the storage.
    *
    * @par dataNode   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    */
    MITKSEMANTICRELATIONS_EXPORT void AddImageInstance(const mitk::DataNode* imageNode, mitk::SemanticRelations& semanticRelationsInstance);

    /*
    * @brief  Add a new segmentation to the storage.
    *
    * @par segmentationNode   The node identifier is extracted from the given segmentation data node.
    * @par parentNode         The case identifier and the node identifier of the parent node is extracted from the given parent data node, which contains DICOM information about the node.
    */
    MITKSEMANTICRELATIONS_EXPORT void AddSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode, mitk::SemanticRelations& semanticRelationsInstance);

    /*
    * @brief  Remove an existing segmentation from the storage.
    *
    * @par segmentationNode   The node identifier is extracted from the given segmentation data node.
    * @par parentNode         The case identifier and the node identifier of the parent node is extracted from the given parent data node, which contains DICOM information about the node.
    */
    MITKSEMANTICRELATIONS_EXPORT void RemoveSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode, mitk::SemanticRelations& semanticRelationsInstance);

  } // namespace SemanticRelationsTestHelper
} // namespace mitk

#endif // MITKSEMANTICRELATIONSTESTHELPER_H
