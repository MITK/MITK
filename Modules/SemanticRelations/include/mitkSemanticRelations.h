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

#ifndef MITKSEMANTICRELATIONS_H
#define MITKSEMANTICRELATIONS_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkControlPointManager.h"
#include "mitkDICOMHelper.h"
#include "mitkRelationStorage.h"
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /*
  * @brief The API provides functions to query and manipulate medical case relations.
  *
  * ##TODO: adjust description
  *   The class is able to generate IDs from given data nodes using DICOM information.
  *   These IDs are used to identify the corresponding instances of a specific case.
  *   The case can also be directly identified by the given case ID.
  *
  *   In the BlackSwan context the case is identified with the DICOM Patient-UID.
  *
  *   In order for most functions to work the case ID has to be existent in the model. If not,
  *   the function does nothing.
  */
  class MITKSEMANTICRELATIONS_EXPORT SemanticRelations
  {
  public:

    SemanticRelations(mitk::DataStorage::Pointer dataStorage);
    ~SemanticRelations();
    std::shared_ptr<RelationStorage> GetRelationStorage();

    /************************************************************************/
    /* functions to get instances / attributes                              */
    /************************************************************************/
    /*
    * @brief  Returns a vector of all lesions that are currently available for the given case.
    *         The lesions may be marked by a segmentation or may be empty - with no connection to a specific image / segmentation of the case data.
    *         If no lesions are stored for the current case, an empty vector is returned.
    *
    * @par caseID    The current case identifier is defined by the given string.
    * @return        A vector of lesions.
    */
    std::vector<SemanticTypes::Lesion> GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID) const;

    /*
    * @brief  Returns a vector of all lesions that are valid for the given case, given a specific lesion
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A specific control point which has to be available at a returned (found) lesion:
    *                     Only those lesions are returned for which the image of the associated segmentation is linked to the given control point.
    *                     If the control point instance does not exist, an empty vector is returned.
    * @return             A vector of control points.
    */
    std::vector<SemanticTypes::Lesion> GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;

    /*
    * @brief  Returns a vector of all lesions that are currently available for the current case and are connected to the given image (via a segmentation).
    *         If no lesions are stored for the current case, an empty vector is returned.
    *
    * @pre    The image node has to have associated segmentation nodes (child nodes) in order to reference a lesion. If no segmentation nodes are
    *         connected with the image node, no lesions for the specific image will be found and an empty vector is returned.
    *
    * @par imageNode    The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return           A vector of lesions.
    */
    std::vector<SemanticTypes::Lesion> GetAllLesionsInImage(const DataNode* imageNode) const;

    /*
    * @brief  Returns the lesion that is defined by the given segmentation data.
    *
    * @pre    Each segmentation has to define a lesion, regardless of a known lesion class. So this function always returns a lesion.
    * @throw  SemanticRelationException, if the segmentation refers to a non-existing lesion (this can be checked via 'IsRepresentingALesion').
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node.
    * @return                 The represented lesion.
    */
    SemanticTypes::Lesion GetRepresentedLesion(const DataNode* segmentationNode) const;

    /*
    * @brief  Check if the given segmentation refers to an existing lesion instance.
    *         This function can be used before calling 'GetRepresentedLesion' in order to avoid a possible exception.
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node.
    * @return                 True, if the segmentation refers to an existing lesion; false otherwise.
    */
    bool IsRepresentingALesion(const DataNode* segmentationNode) const;

    /*
    * @brief  Return a vector of all segmentations that define the given lesion. These segmentations don't have to be linked to the same image.
    *         If the lesion is not referred to by any segmentation, an empty vector is returned.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   A Lesion with a UID that identifies the corresponding lesion instance.
    * @return       A vector of data nodes representing segmentations that define the given lesion.
    */
    std::vector<DataNode::Pointer> GetAllSegmentationsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;

    /*
    * @brief  Check if the given lesion instance exists.
    *         This function can be used before calling 'GetAllSegmentationsOfLesion' in order to avoid a possible exception.
    *         This function can be used before calling 'AddLesionInstance' in order to avoid a possible exception.
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   A Lesion with a UID that identifies the corresponding lesion instance.
    * @return       True, if the lesion instance exists; false otherwise.
    */
    bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;

    /*
    * @brief Return a vector of all control points that are valid for the given case.
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @return               A vector of control points.
    */
    std::vector<SemanticTypes::ControlPoint> GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID) const;

    /*
    * @brief Return a vector of all control points that are valid for the given case, given a specific lesion
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @par lesion           A specific lesion which has to be available at a returned (found) control point:
    *                       Only those control points are returned for which an associated data has a segmentation that references the given lesion.
    *                       If the lesion does not exists, an empty vector is returned.
    * @return               A vector of control points.
    */
    std::vector<SemanticTypes::ControlPoint> GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;

    /*
    * @brief Return a vector of all control points that are valid for the given case, given a specific information type.
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @par informationType  A specific information type which has to be available at a returned (found) control point:
    *                       Only those control points are returned for which an associated data has the given information type.
    *                       If the information type instance does not exists, an empty vector is returned.
    * @return               A vector of control points.
    */
    std::vector<SemanticTypes::ControlPoint> GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const;

    /*
    * @brief  Return the control point of a data node.
    *         If the data node is not linked to a control point or the data node refers to a non-existing control point,
    *         a control point with an empty UID is returned.
    *
    * @par dataNode   The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return         The control point of the given data node.
    */
    SemanticTypes::ControlPoint GetControlPointOfData(const DataNode* dataNode) const;

    /*
    * @brief  Return a vector of all data nodes that link to the given control point.
    *         If the control point is not referred to by any data node, an empty vector is returned.
    *
    * @pre    The UID of the control point has to exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point does not exist for a control point instance (this can be checked via 'InstanceExists').
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    * @return             A vector of data nodes that link to the given control point.
    */
    std::vector<DataNode::Pointer> GetAllDataOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;

    /*
    * @brief  Check if the given control point instance exists.
    *         This function can be used before calling 'GetAllDataOfControlPoint' in order to avoid a possible exception.
    *         This function can be used before adding, linking and unlinking control points to avoid a possible exception.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    * @return             True, if the control point instance exists; false otherwise.
    */
    bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;

    /*
    * @brief  Return a vector of all information types that are valid for the given case.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @return             A vector of information types.
    */
    std::vector<SemanticTypes::InformationType> GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID) const;

    /*
    * @brief  Return a vector of all information types that are valid for the given case, given a specific control point.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A specific control point which has to be available at a returned (found) information type:
    *                     Only those information types are returned for which an associated data is linked to the given control point.
    *                     If the control point instance does not exist, an empty vector is returned.
    * @return             A vector of information types.
    */
    std::vector<SemanticTypes::InformationType> GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;

    /*
    * @brief  Return the information type of the given image.
    *         If the image does not contain any information type, an empty information type is returned.
    *
    * @par imageNode    The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return           The information type of the given data node.
    */
    SemanticTypes::InformationType GetInformationTypeOfImage(const DataNode* imageNode) const;

    /*
    * @brief  Return a vector of all data nodes that are defined as the given information type.
    *
    * @pre    The information type has to exist for the given case (and is therefore used by at least one data node).
    * @throw  SemanticRelationException, if the information type is not used by any data node (this can be checked via 'InstanceExists').
    *
    * @par caseID             The current case identifier is defined by the given string.
    * @par informationType    An information type that identifies the corresponding information type instance.
    * @return                 A vector of data nodes that are defined as the given information type.
    */
    std::vector<mitk::DataNode::Pointer> GetAllDataOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const;

    /*
    * @brief  Return a vector of all data nodes that are defined as the given information type with the given control point.

    * @pre    The UID of the control point has to exist for a control point instance.
    *         The information type has to exist for the given case (and is therefore used by at least one data node).
    * @throw  SemanticRelationException, if the UID of the control point does not exist for a control point instance (this can be checked via 'InstanceExists') or
    *                                    if the information type is not used by any data node (this can be checked via 'InstanceExists')
    *
    * @par caseID             The current case identifier is defined by the given string.
    * @par controlPoint       A control point with a UID that identifies the corresponding control point instance.
    * @par informationType    An information type that identifies the corresponding information type instance.
    * @return                 A vector of data nodes that are defined as the given information type with the given control point.
    */
    std::vector<mitk::DataNode::Pointer> GetFilteredData(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const;

    /*
    * @brief  Check if the given information type exists.
    *         This function can be used before calling 'GetAllDataOfInformationType' in order to avoid a possible exception.
    *
    * @par caseID             The current case identifier is defined by the given string.
    * @par informationType    An information type
    * @return                 True, if the information type exists; false otherwise.
    */
    bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const;

    /*
    * @brief  Return a vector of all CaseIDs that are currently available.
    *
    * @return       A vector of CaseIDs as strings.
    */
    std::vector<SemanticTypes::CaseID> GetAllCaseIDs() const;

    /************************************************************************/
    /* functions to add / remove instances / attributes                     */
    /************************************************************************/

    /*
    * @brief  Add a newly created lesion to the set of already existing lesions - with no connection to a specific image / segmentation of the case data.
    *
    * @pre    The UID of the lesion must not already exist for a lesion instance.
    * @throw  SemanticRelationException, it the UID of the lesion already exists for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   The lesion instance to add.
    */
    void AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);

    /*
    * @brief  Overwrite an already existing lesion instance (this may be useful to overwrite the lesion with a different lesion class).
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   The lesion instance that overwrites an existing lesion.
    */
    void OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);

    /*
    * @brief  Add a newly created lesion to the set of already existing lesions. The lesion is added and a reference to
    *         the lesion is added to the segmentation data. If the segmentation is already linked to a lesion, the
    *         old linkage is overwritten (this can be checked via 'IsRepresentingALesion').
    *
    * @pre    The UID of the lesion must not already exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion already exists for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    * @par lesion             The lesion instance to add and link.
    */
    void AddLesionAndLinkData(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion);

    /*
    * @brief  Link the given segmentation instance to an an already existing lesion instance. If the segmentation is already linked to a lesion instance, the
    *         old linkage is overwritten (this can be checked via 'IsRepresentingALesion').
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    * @par lesion             The lesion instance to link.
    */
    void LinkSegmentationToLesion(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion);

    /*
    * @brief  Unlink the given segmentation instance from the linked lesion instance.
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    */
    void UnlinkSegmentationFromLesion(const DataNode* segmentationNode);

    /*
    * @brief  Remove the given lesion from the set of already existing lesions.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    *         If the lesion instance should be removed, the function needs to assure that no segmentation is still representing (linked to) this lesion.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    * @throw  SemanticRelationException, if the lesion instance to remove is still linked to by any segmentation (this can be checked via 'GetAllSegmentationsOfLesion').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   The lesion instance to remove.
    */
    void RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);

    /*
    * @brief  Add a newly created control point to the set of already existing control points. A reference to the control point is added to the given data.
    *         This function combines adding a control point and linking it, since a control point with no associated data is not allowed.
    *
    * @pre    The UID of the control point must not already exist for a control point instance.
    *         The function checks, if the given control point is already contained in an existing control point interval.
    *         To add a new control point, the control point is always expected to have "startDate = endDate".
    * @throw  SemanticRelationException, if the UID of the control point already exists for a control point instance (this can be checked via 'InstanceExists').
    * @throw  SemanticRelationException, if the given control point is already contained in an existing control point interval (this can be checked via 'CheckContainingControlPoint').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    *
    * @par dataNode         The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @par controlPoint     The control point instance to add. For a newly added control point always has "startDate = endDate".
    * @par checkConsistence If true, the function checks, whether the date of the data node actually lies inside the control point to link.
    */
    void AddControlPointAndLinkData(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence = true);

    /*
    * @brief  Link the given data to an already existing control point and overwrite the start or end point of the control point.
    *
    *@pre    The UID of the control point has to exist for a control point instance.
    *        The function assures that the date of the data lies inside the time period of the control point.
    *        The function assures that the overwritten control point instance is extended / shortened to its necessary time period
    *        (e.g.moving the start point to an earlier date) and does not overlap with an already existing control point.
    * @throw  SemanticRelationException, if the UID of the control point does not exists for a control point instance (this can be checked via 'InstanceExists').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    * @throw  SemanticRelationException, if the given control point does not differ from the overwritten control point or if the given control point differs in the start date and the end date from the overwritten control point.
    * @throw  SemanticRelationException, if the given control point overlaps with an already existing control point interval (this can be checked via 'CheckOverlappingControlPoint').
    *
    * @par dataNode         The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @par controlPoint     The control point instance that overwrites an existing control point.
    * @par checkConsistence If true, the function checks, whether the date of the data node actually lies inside the control point to link.
    */
    void OverwriteControlPointAndLinkData(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence = true);

    /*
    * @brief  Link the given data to an already existing control point.
    *
    * @pre    The UID of the control point has to exist for a control point instance
    *         The function assures that the date of the data lies inside the time period of the control point.
    * @throw  SemanticRelationException, if the UID of the control point does not exists for a control point instance (this can be checked via 'InstanceExists').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    *
    * @par dataNode         The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @par controlPoint     The control point instance to add link.
    * @par checkConsistence If true, the function checks, whether the date of the data node actually lies inside the control point to link.
    */
    void LinkDataToControlPoint(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence = true);

    /*
    * @brief  Unlink the given image from the linked control point.
    *
    * @pre    If data is unlinked from a control point, the function needs to check whether the control point is still linked to any other data:
    *           - if not, the control point instance can be removed (has to be removed since a control point with no associated data is not allowed).
    *           - if so, the function has to make sure that the control point instance is shortened to its minimum time period (e.g. moving the end point to an earlier date).
    * @throw  SemanticRelationException, if the linked control point of the data is not correctly stored (this can be checked via 'InstanceExists').
    *
    * @par dataNode       The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    */
    void UnlinkDataFromControlPoint(const DataNode* dataNode);

    /*
    * @brief Set the information type of the given image.
    *
    * @post  If the information type instance did not exist before, it is now added.
    *
    * @par imageNode        The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @par informationType  An information type that identifies the corresponding information type instance.
    */
    void AddInformationTypeToImage(const DataNode* imageNode, const SemanticTypes::InformationType& informationType);

    /*
    * @brief  Remove the information type of the given image.
    *
    * @pre    If the information type is removed, the function needs to check whether the information type is referenced by any other image:
    *           - if not, the information type instance can be removed (has to be removed since an information type with no associated image is not allowed).
    *           - if so, the information type is just removed from the given image.
    *
    * @par imageNode        The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    */
    void RemoveInformationTypeFromImage(const DataNode* imageNode);

  private:

    // the relation storage serves as a storage accessor and can be sub-classed for custom demands
    std::shared_ptr<RelationStorage> m_RelationStorage;
    DataStorage::Pointer m_DataStorage;

    /*
    * @brief
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    */
    bool CheckOverlappingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);

    /*
    * @brief
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    */
    bool CheckContainingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
  };

} // namespace mitk

#endif // MITKSEMANTICRELATIONS_H
