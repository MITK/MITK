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

#ifndef MITKSEMANTICRELATIONSINTEGRATION_H
#define MITKSEMANTICRELATIONSINTEGRATION_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkISemanticRelationsObservable.h"
#include "mitkISemanticRelationsObserver.h"
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>

namespace mitk
{
  /**
  * @brief  The API provides functions to manipulate image relations and instances
  *         that are helpful during follow-up examination, like control-points (time period),
  *         types of the images or lesions that may be visible on multiple images.
  *
  *   The class is able to generate IDs from given data nodes using DICOM information.
  *   These IDs are used to identify the corresponding instances of a specific case.
  *   The case can also be directly identified by the given case ID.
  *
  *   In order for most functions to work the case ID has to be used as a parameter.
  *   If not, these functions do nothing.
  *
  *   The class implements the ISemanticRelationsObservable interface to allow observers to 
  *   be informed about changes in the semantic relation storage.
  */
  class MITKSEMANTICRELATIONS_EXPORT SemanticRelationsIntegration : public ISemanticRelationsObservable
  {
  public:

    /************************************************************************/
    /* functions to implement the observer pattern                          */
    /************************************************************************/
    /**
    * @brief Adds the given concrete observer to the vector that holds all currently registered observer.
    *        If the observer is already registered, it will not be added to the observer vector.
    *
    * @param observer   The concrete observer to register.
    */
    virtual void AddObserver(ISemanticRelationsObserver* observer) override;
    /**
    * @brief Removes the given concrete observer from the vector that holds all currently registered observer.
    *
    * @param observer   The concrete observer to unregister.
    */
    virtual void RemoveObserver(ISemanticRelationsObserver* observer) override;
  
    /************************************************************************/
    /* functions to add / remove instances / attributes                     */
    /************************************************************************/
    /**
    * @brief  Add the given image to the set of already existing images.
    *         The date is extracted from the DICOM data of the image node and is compared to already existing control points in the semantic relations model.
    *         The function tries to find a fitting control point or to extend an already existing control point, if the extracted control point is close to
    *         any other, already existing control point.
    *         Finally, the image is linked to the correct control point.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param imageNode   The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    */
    void AddImage(const DataNode* imageNode);
    /**
    * @brief  Remove the given image from the set of already existing images.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param imageNode   The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    */
    void RemoveImage(const DataNode* imageNode);
    /**
    * @brief  Add a newly created lesion to the set of already existing lesions - with no connection to a specific image / segmentation of the case data.
    *
    * @pre    The UID of the lesion must not already exist for a lesion instance.
    * @throw  SemanticRelationException, it the UID of the lesion already exists for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   The lesion instance to add.
    */
    void AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Overwrite an already existing lesion instance (this may be useful to overwrite the lesion with a different lesion class).
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   The lesion instance that overwrites an existing lesion.
    */
    void OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Add a newly created lesion to the set of already existing lesions. The lesion is added and a reference to
    *         the lesion is added to the segmentation. If the segmentation is already linked to a lesion, the
    *         old linkage is overwritten (this can be checked via 'IsRepresentingALesion').
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given segmentation data node is invalid (==nullptr).
    * @pre    The UID of the lesion must not already exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion already exists for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given segmentation data node. The segmentation node has DICOM information from its parent node.
    * @param lesion             The lesion instance to add and link.
    */
    void AddLesionAndLinkSegmentation(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Remove the given lesion from the set of already existing lesions.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    * @pre    The function needs to assure that no segmentation is still representing (linked to) this lesion.
    * @throw  SemanticRelationException, if the lesion instance to remove is still linked to by any segmentation (this can be checked via 'GetAllSegmentationsOfLesion').
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   The lesion instance to remove.
    */
    void RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Add a segmentation instance to the set of already existing segmentations - with no connection to a specific lesion.
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given segmentation data node. The segmentation node has DICOM information from its parent node.
    * @param parentNode         The node identifier of the parent node is extracted from the given parent data node.
    */
    void AddSegmentation(const DataNode* segmentationNode, const DataNode* parentNode);
    /**
    * @brief  Link the given segmentation instance to an an already existing lesion instance. If the segmentation is already linked to a lesion instance, the
    *         old linkage is overwritten (this can be checked via 'IsRepresentingALesion').
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given segmentation data node is invalid (==nullptr).
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given segmentation data node. The segmentation node has DICOM information from its parent node.
    * @param lesion             The lesion instance to link.
    */
    void LinkSegmentationToLesion(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Unlink the given segmentation instance from the linked lesion instance.
    *         The lesion may stay unlinked to any segmentation.
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given segmentation data node is invalid (==nullptr).
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given segmentation data node. The segmentation node has DICOM information from its parent node.
    */
    void UnlinkSegmentationFromLesion(const DataNode* segmentationNode);
    /**
    * @brief  Remove the given segmentation from the set of already existing segmentations.
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given segmentation data node is invalid (==nullptr).
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given segmentation data node. The segmentation node has DICOM information from its parent node.
    */
    void RemoveSegmentation(const DataNode* segmentationNode);
    /**
    * @brief Set the control point for the given image.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param imageNode      The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    * @param controlPoint   The control point instance which is used for the given image.
    */
    void SetControlPointOfImage(const DataNode* imageNode, const SemanticTypes::ControlPoint& controlPoint);
    /**
    * @brief  Add a newly created control point to the set of already existing control points. A reference to the control point is added to the given image.
    *         This function combines adding a control point and linking it, since a control point with no associated data is not allowed.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    * @pre    The UID of the control point must not already exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point already exists for a control point instance (this can be checked via 'InstanceExists').
    * @pre    The given control point must not already be contained in an existing control point interval.
    * @throw  SemanticRelationException, if the given control point is already contained in an existing control point interval (this can be checked via 'CheckContainingControlPoint').
    * @pre    The given control point must contain the date of the given image data node (if parameter 'checkConsistence = true').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given image data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    *
    * @param imageNode        The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    * @param controlPoint     The control point instance to add. For a newly added control point always has "startDate = endDate".
    * @param checkConsistence If true, the function checks, whether the date of the image data node actually lies inside the control point to link.
    */
    void AddControlPointAndLinkImage(const DataNode* imageNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence = true);
    /**
    * @brief  Link the given image to an already existing control point.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    * @pre    The UID of the control point has to exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point does not exists for a control point instance (this can be checked via 'InstanceExists').
    * @pre    The given control point must contain the date of the given image data node (if parameter 'checkConsistence = true').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given image data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    *
    * @param imageNode        The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    * @param controlPoint     The control point instance to link.
    * @param checkConsistence If true, the function checks, whether the date of the image data node actually lies inside the control point to link.
    */
    void LinkImageToControlPoint(const DataNode* imageNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence = true);
    /**
    * @brief  Unlink the given image from the linked control point.
    *         If an image is unlinked from a control point, the function needs to check whether the control point is still linked to any other image:
    *           - if not, the control point instance will be removed (has to be removed since a control point with no associated image is not allowed).
    *           - if so, the function has to make sure that the control point instance is shortened to its minimum time period (e.g. moving the end point to an earlier date).
    *
    * @param imageNode        The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    */
    void UnlinkImageFromControlPoint(const DataNode* imageNode);
    /**
    * @brief  Add an examination period instance to the set of already existing examination periods - with no connection to a specific control point.
    *
    * @pre    The UID of the examination period must not already exist for an examination period instance.
    * @throw  SemanticRelationException, if the UID of the examination period already exists for a examination period instance (this can be checked via 'InstanceExists').
    *
    * @param caseID               The current case identifier is defined by the given string.
    * @param examinationPeriod    The examination period to add.
    */
    void AddExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    /**
    * @brief Add a control point to the vector of control point UIDs of an existing examination period.
    *
    * @pre    The UID of the control point has to exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point does not exists for a control point instance (this can be checked via 'InstanceExists').
    * @pre    The UID of the examination period must not already exist for an examination period instance.
    * @throw  SemanticRelationException, if the UID of the examination period already exists for a examination period instance (this can be checked via 'InstanceExists').
    *
    * @param caseID               The current case identifier is defined by the given string.
    * @param controlPoint         The control point instance to add to the examination period.
    * @param examinationPeriod    The examination period to which the control point should be added.
    */
    void AddControlPointToExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    /**
    * @brief Set (and possibly overwrite) the information type of the given image.
    *        An already associated information type might be removed if is not referenced by any other image:
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    * @post   If the information type instance did not exist before, it is now added.
    *
    * @param imageNode        The current case identifier is extracted from the given image data node, which contains DICOM information about the case.
    * @param informationType  An information type that identifies the corresponding information type instance.
    */
    void SetInformationType(const DataNode* imageNode, const SemanticTypes::InformationType& informationType);
    /**
    * @brief  Set the information type of the given image.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    * @post   If the information type instance did not exist before, it is now added.
    *
    * @param imageNode        The current case identifier is extracted from the given image data node, which contains DICOM information about the case.
    * @param informationType  An information type that identifies the corresponding information type instance.
    */
    void AddInformationTypeToImage(const DataNode* imageNode, const SemanticTypes::InformationType& informationType);
    /**
    * @brief  Remove the information type of the given image.
    *         If the information type is removed, the function needs to check whether the information type is referenced by any other image:
    *           - if not, the information type instance can be removed (has to be removed since an information type with no associated image is not allowed).
    *           - if so, the information type is just removed from the given image.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param imageNode        The current case identifier is extracted from the given image data node, which contains DICOM information about the case.
    */
    void RemoveInformationTypeFromImage(const DataNode* imageNode);

  private:
    /**
    * @brief A vector that stores the currently registered observer of this observable subject.
    */
    static std::vector<mitk::ISemanticRelationsObserver*> m_ObserverVector;
    /**
    * @brief The class notifies (updates) the observer with a given case ID.
    *        The view's caseID was set before in the GUI. The parts of the view that observe changes in the semantic relations are only updated,
    *        if the given case ID is equal to the observer's current caseID and thus the observer currently shows the semantic information of the given case.
    *
    * @param  caseID    The caseID that identifies the currently active patient / case.
    */
    virtual void NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) const override;
    /**
    * @brief Remove all control points from the storage that are not referenced by any image anymore.
    *        This might happen if an image has been removed (and unlinked from the corresponding control point)
    *        or if the user sets a new control point for an image manually in the GUI.
    *
    * @param caseID   The current case identifier is defined by the given string.
    */
    void ClearControlPoints(const SemanticTypes::CaseID& caseID);

  };
} // namespace mitk

#endif // MITKSEMANTICRELATIONSINTEGRATION_H
