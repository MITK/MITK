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

    /*
    * @brief Adds the given concrete observer to the vector that holds all currently registered observer.
    *        If the observer is already registered, it will not be added to the observer vector.
    *
    * @par observer   The concrete observer to register.
    */
    virtual void AddObserver(ISemanticRelationsObserver* observer) override;
    /*
    * @brief Removes the given concrete observer from the vector that holds all currently registered observer.
    *
    * @par observer   The concrete observer to unregister.
    */
    virtual void RemoveObserver(ISemanticRelationsObserver* observer) override;
    /*
    * @brief Sets the current caseID for the semantic relations. The caseID determines the currently active
    *        patient (case) and can be changed via the GUI by the user.
    *        If the caseID is set, all observers are notified so that they update their view to show the new,
    *        active data of the patient (case).
    */
    void SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID);

    /************************************************************************/
    /* functions to add/remove image and segmentation instances				      */
    /************************************************************************/
    /*
    * @brief  Adds a new image to the storage.
    *
    * @par dataNode   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    */
    void AddImageInstance(const mitk::DataNode* imageNode);
    /*
    * @brief  Removes an existing image from the storage.
    *
    * @par dataNode   The current case identifier and node identifier is extracted from the given data node, which contains DICOM information about the case and the node.
    */
    void RemoveImageInstance(const mitk::DataNode* imageNode);
    /*
    * @brief  Links (and possibly adds) a segmentation to a lesion inside the storage.
    *
    * @par segmentationNode   The current case identifier and node identifier is extracted from the given segmentation data node.
    * @par parentNode         The node identifier of the parent node is extracted from the given parent data node.
    * @par selectedLesion    The selected lesion that should be linked with the given segmentation.
    */
    void AddAndLinkSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode, const SemanticTypes::Lesion& selectedLesion);
    /*
    * @brief  Unlinks (and possibly removes) a segmentation from a lesion inside the storage.
    *
    * @par segmentationNode   The current case identifier and node identifier is extracted from the given segmentation data node.
    */
    void RemoveAndUnlinkSegmentationInstance(const mitk::DataNode* segmentationNode);
    /*
    * @brief  Generates a new, empty lesion to add to the storage under the current case ID.
    *
    * @par caseID   The current case identifier is defined by the given string.
    */
    void GenerateNewLesion(const mitk::SemanticTypes::CaseID caseID);
    /*
    * @brief  Remove a selected lesion from the storage.
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @par selectedLesion   The selected lesion that should be removed.
    */
    void RemoveLesion(const mitk::SemanticTypes::CaseID caseID, const SemanticTypes::Lesion& selectedLesion);

  private:
    /*
    * @brief The SemanticRelationsManager, as an example of an observable subject, only notifies (updates) the observer, if
    *        the caseID is equal to its own current caseID. The own caseID was set before by the view, so that the parts of the view
    *        that observe changes in the semantic relations, are only updates if they currently show the semantic information of the given case.
    *
    * @par  caseID    The caseID that identifies the currently active patient / case.
    */
    virtual void NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) const override;

    std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
    /*
    * @brief The current caseID that is used to determine if observer should be notified.
    *        The caseID determines the currently active patient (case).
    */
    mitk::SemanticTypes::CaseID m_CaseID;
    /*
    * @brief A vector that stores the currently registered observer of this observable subject.
    */
    std::vector<mitk::ISemanticRelationsObserver*> m_ObserverVector;

  }; // class SemanticRelationsManager
} // namespace mitk

#endif // MITKSEMANTICRELATIONSMANAGER_H
