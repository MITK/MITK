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

#ifndef MITKSEMANTICRELATIONSMANAGER_H
#define MITKSEMANTICRELATIONSMANAGER_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkSemanticRelations.h"

namespace mitk
{
  class SemanticRelationsManager
  {
  public:
    static SemanticRelationsManager* SemanticRelationsManager::GetInstance();

    bool SemanticRelationsManager::IsInstantiated();

    SemanticRelationsManager(const SemanticRelationsManager&) = delete;
    void operator=(const SemanticRelationsManager&) = delete;

    /************************************************************************/
    /* functions to add/remove image and segmentation instances				      */
    /************************************************************************/
    /*
    * @brief  Add a new image to the storage.
    *
    * @par dataNode                   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    * @par semanticRelationsInstance  An instance of the currently used semantic relations class.
    */
    MITKSEMANTICRELATIONS_EXPORT void AddImageInstance(const mitk::DataNode* imageNode, mitk::SemanticRelations& semanticRelationsInstance);
    /*
    * @brief  Remove an existing image from the storage.
    *
    * @par dataNode                   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    * @par semanticRelationsInstance  An instance of the currently used semantic relations class.
    */
    MITKSEMANTICRELATIONS_EXPORT void RemoveImageInstance(const mitk::DataNode* imageNode, mitk::SemanticRelations& semanticRelationsInstance);
    /*
    * @brief  Add a new segmentation to the storage.
    *
    * @par segmentationNode           The current case identifier and node identifier is extracted from the given segmentation data node.
    * @par parentNode                 The node identifier of the parent node is extracted from the given parent data node.
    * @par semanticRelationsInstance  An instance of the currently used semantic relations class.
    */
    MITKSEMANTICRELATIONS_EXPORT void AddSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode, mitk::SemanticRelations& semanticRelationsInstance);
    /*
    * @brief  Remove an existing segmentation from the storage.
    *
    * @par segmentationNode           The current case identifier and node identifier is extracted from the given segmentation data node.
    * @par semanticRelationsInstance  An instance of the currently used semantic relations class.
    */
    MITKSEMANTICRELATIONS_EXPORT void RemoveSegmentationInstance(const mitk::DataNode* segmentationNode, mitk::SemanticRelations& semanticRelationsInstance);

  private:
    SemanticRelationsManager() {}

  } // class SemanticRelationsManager
} // namespace mitk

#endif // MITKSEMANTICRELATIONSMANAGER_H
