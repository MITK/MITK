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
#include "mitkISemanticRelationsObservable.h"
#include "mitkISemanticRelationsObserver.h"
#include "mitkSemanticRelations.h"

// mitk core
#include <mitkDataStorage.h>

namespace mitk
{
  class MITKSEMANTICRELATIONS_EXPORT SemanticRelationsManager : public ISemanticRelationsObservable
  {
  public:

    SemanticRelationsManager(mitk::DataStorage::Pointer dataStorage);
    ~SemanticRelationsManager();

    virtual void AddObserver(ISemanticRelationsObserver* observer) override;
    virtual void RemoveObserver(ISemanticRelationsObserver* observer) override;

    void SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID);

    /************************************************************************/
    /* functions to add/remove image and segmentation instances				      */
    /************************************************************************/
    /*
    * @brief  Add a new image to the storage.
    *
    * @par dataNode   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    */
    void AddImageInstance(const mitk::DataNode* imageNode);
    /*
    * @brief  Remove an existing image from the storage.
    *
    * @par dataNode   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    */
    void RemoveImageInstance(const mitk::DataNode* imageNode);
    /*
    * @brief  Add a new segmentation to the storage.
    *
    * @par segmentationNode   The current case identifier and node identifier is extracted from the given segmentation data node.
    * @par parentNode         The node identifier of the parent node is extracted from the given parent data node.
    */
    void AddSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode);
    /*
    * @brief  Remove an existing segmentation from the storage.
    *
    * @par segmentationNode   The current case identifier and node identifier is extracted from the given segmentation data node.
    */
    void RemoveSegmentationInstance(const mitk::DataNode* segmentationNode);
    /*
    * @brief  Generate a new, empty lesion to add to the storage under the current case ID.
    *
    * @par caseID   The current case identifier is defined by the given string.
    */
    void GenerateNewLesion(const mitk::SemanticTypes::CaseID caseID);

  private:

    virtual void NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) override;

    std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
    mitk::SemanticTypes::CaseID m_CaseID;
    std::vector<mitk::ISemanticRelationsObserver*> m_ObserverVector;

  }; // class SemanticRelationsManager
} // namespace mitk

#endif // MITKSEMANTICRELATIONSMANAGER_H
