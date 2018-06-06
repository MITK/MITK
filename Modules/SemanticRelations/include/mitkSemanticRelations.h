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
#include "mitkISemanticRelationsObservable.h"
#include "mitkISemanticRelationsObserver.h"
#include "mitkRelationStorage.h"
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /*
  * @brief  The API provides functions to query and manipulate image relations and instances,
  *         that are helpful during follow-up examination, like control-points (time period),
  *         types of the images or lesions that may be visible on multiple images.
  *
  *   The class is able to generate IDs from given data nodes using DICOM information.
  *   These IDs are used to identify the corresponding instances of a specific case.
  *   The case can also be directly identified by the given case ID.
  *
  *   In the BlackSwan context the case is identified with the DICOM PatientID.
  *
  *   In order for most functions to work the case ID has to be defined in the model. If not,
  *   the functions do nothing.
  */
  class MITKSEMANTICRELATIONS_EXPORT SemanticRelations : public ISemanticRelationsObservable
  {
  public:

    SemanticRelations(mitk::DataStorage::Pointer dataStorage);
    ~SemanticRelations();

    typedef std::vector<SemanticTypes::Lesion> LesionVector;
    typedef std::vector<SemanticTypes::ControlPoint> ControlpointVector;
    typedef std::vector<SemanticTypes::InformationType> InformationTypeVector;
    typedef std::vector<DataNode::Pointer> DataNodeVector;
    /************************************************************************/
    /* functions to implement the observer pattern                          */
    /************************************************************************/
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
    LesionVector GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID) const;
    /*
    * @brief  Returns a vector of all lesions that are valid for the given case, given a specific lesion
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A specific control point which has to be available at a returned (found) lesion:
    *                     Only those lesions are returned for which the image of the associated segmentation is linked to the given control point.
    *                     If the control point instance does not exist, an empty vector is returned.
    * @return             A vector of control points.
    */
    LesionVector GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;
    /*
    * @brief  Returns a vector of all lesions that are currently available for the current case and are connected to the given image (via a segmentation).
    *         If no lesions are stored for the current case, an empty vector is returned. If no segmentation nodes are
    *         connected with the image node, no lesions for the specific image will be found and an empty vector is returned.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given image data node is invalid (==nullptr).
    * @pre    The image node has to have associated segmentation nodes (child nodes) in order to reference a lesion.
    *
    * @par imageNode    The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return           A vector of lesions.
    */
    LesionVector GetAllLesionsInImage(const DataNode* imageNode) const;
    /*
    * @brief  Returns the lesion that is defined by the given segmentation data.
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given segmentation data node is invalid (==nullptr).
    * @pre    The segmentation data node has to represent a lesion. If not, the retrieved lesion will be empty, which leads to an exception.
    * @throw  SemanticRelationException, if the segmentation does not represent an existing lesion (this can be checked via 'IsRepresentingALesion').
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
    * @brief  Return a vector of all segmentations that are currently available for the given case.
    *         The segmentations may be connected / not connected to a lesion of the case.
    *         If no segmentations are stored for the current case, an empty vector is returned.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  mitk::Exception if the data storage member is invalid (==nullptr).
    *
    * @par caseID    The current case identifier is defined by the given string.
    * @return        A vector of data nodes representing segmentations.
    */
    mitk::SemanticRelations::DataNodeVector GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID) const;
    /*
    * @brief  Return a vector of all segmentations that define the given lesion. These segmentations don't have to be linked to the same image.
    *         If the lesion is not referred to by any segmentation, an empty vector is returned.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  mitk::Exception if the data storage member is invalid (==nullptr).
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   A Lesion with a UID that identifies the corresponding lesion instance.
    * @return       A vector of data nodes representing segmentations that define the given lesion.
    */
    DataNodeVector GetAllSegmentationsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;
    /*
    * @brief  Return a vector of all images that are currently available for the given case.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  mitk::Exception if the data storage member is invalid (==nullptr).
    *
    * @par caseID    The current case identifier is defined by the given string.
    * @return        A vector of data nodes representing images.
    */
    DataNodeVector GetAllImagesOfCase(const SemanticTypes::CaseID& caseID) const;
    /*
    * @brief Return a vector of all images that are connected to those segmentations that are linked to the given lesion.
    *         If the lesion is not referred to by any segmentation, an empty vector is returned.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   A Lesion with a UID that identifies the corresponding lesion instance.
    * @return       A vector of data nodes representing images on which the lesions are visible.
    */
    DataNodeVector GetAllImagesOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;
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
    ControlpointVector GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID) const;
    /*
    * @brief Return a vector of all control points that are valid for the given case, given a specific lesion
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @par lesion           A specific lesion which has to be available at a returned (found) control point:
    *                       Only those control points are returned for which an associated data has a segmentation that references the given lesion.
    *                       If the lesion does not exists, an empty vector is returned.
    * @return               A vector of control points.
    */
    ControlpointVector GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;
    /*
    * @brief Return a vector of all control points that are valid for the given case, given a specific information type.
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @par informationType  A specific information type which has to be available at a returned (found) control point:
    *                       Only those control points are returned for which an associated data has the given information type.
    *                       If the information type instance does not exists, an empty vector is returned.
    * @return               A vector of control points.
    */
    ControlpointVector GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const;
    /*
    * @brief  Return the control point of a data node.
    *         If the data node is not linked to a control point or the data node refers to a non-existing control point,
    *         a control point with an empty UID is returned.
    *
    * @pre    The given data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given data node is invalid (==nullptr).
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
    DataNodeVector GetAllDataOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;
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
    InformationTypeVector GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID) const;
    /*
    * @brief  Return a vector of all information types that are valid for the given case, given a specific control point.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A specific control point which has to be available at a returned (found) information type:
    *                     Only those information types are returned for which an associated data is linked to the given control point.
    *                     If the control point instance does not exist, an empty vector is returned.
    * @return             A vector of information types.
    */
    InformationTypeVector GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const;
    /*
    * @brief  Return the information type of the given image.
    *         If the image does not contain any information type, an empty information type is returned.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given image data node is invalid (==nullptr).
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
    DataNodeVector GetAllDataOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const;
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
    DataNodeVector GetFilteredData(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const;
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
    * @brief  Add the given image to the set of already existing images.
    *         The date is extracted from the DICOM data of the image node and is compared to already existing control points in the semantic relations model.
    *         The function tries to find a fitting control point or to extend an already existing control point, if the extracted control point is close to
    *         any other, already existing control point.
    *         Finally, the image is linked to the correct control point.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given image data node is invalid (==nullptr).
    *
    * @par imageNode   The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    */
    void AddImage(const DataNode* imageNode);
    /*
    * @brief  Remove the given image from the set of already existing images.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given image data node is invalid (==nullptr).
    *
    * @par imageNode   The current case identifier and node identifier is extracted from the given image data node, which contains DICOM information about the case and the node.
    */
    void RemoveImage(const DataNode* imageNode);
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
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given segmentation data node is invalid (==nullptr).
    * @pre    The UID of the lesion must not already exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion already exists for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    * @par lesion             The lesion instance to add and link.
    */
    void AddLesionAndLinkSegmentation(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion);
    /*
    * @brief  Remove the given lesion from the set of already existing lesions.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    * @pre    The function needs to assure that no segmentation is still representing (linked to) this lesion.
    * @throw  SemanticRelationException, if the lesion instance to remove is still linked to by any segmentation (this can be checked via 'GetAllSegmentationsOfLesion').
    *
    * @par caseID   The current case identifier is defined by the given string.
    * @par lesion   The lesion instance to remove.
    */
    void RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /*
    * @brief  Add a segmentation instance to the set of already existing segmentations - with no connection to a specific lesion.
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    * @par parentNode         The node identifier of the parent node is extracted from the given parent data node.
    */
    void AddSegmentation(const DataNode* segmentationNode, const DataNode* parentNode);
    /*
    * @brief  Link the given segmentation instance to an an already existing lesion instance. If the segmentation is already linked to a lesion instance, the
    *         old linkage is overwritten (this can be checked via 'IsRepresentingALesion').
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given segmentation data node is invalid (==nullptr).
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if the UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    * @par lesion             The lesion instance to link.
    */
    void LinkSegmentationToLesion(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion);
    /*
    * @brief  Unlink the given segmentation instance from the linked lesion instance.
    *         The lesion may stay unlinked to any segmentation.
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given segmentation data node is invalid (==nullptr).
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    */
    void UnlinkSegmentationFromLesion(const DataNode* segmentationNode);
    /*
    * @brief  Remove the given segmentation from the set of already existing segmentations.
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given segmentation data node is invalid (==nullptr).
    *
    * @par segmentationNode   The segmentation identifier is extracted from the given data node. The segmentation node has DICOM information from its parent node.
    */
    void RemoveSegmentation(const DataNode* segmentationNode);
    /*
    * @brief  Add a newly created control point to the set of already existing control points. A reference to the control point is added to the given data.
    *         This function combines adding a control point and linking it, since a control point with no associated data is not allowed.
    *
    * @pre    The given data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given data node is invalid (==nullptr).
    * @pre    The UID of the control point must not already exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point already exists for a control point instance (this can be checked via 'InstanceExists').
    * @pre    The given control point must not already be contained in an existing control point interval.
    * @throw  SemanticRelationException, if the given control point is already contained in an existing control point interval (this can be checked via 'CheckContainingControlPoint').
    * @pre    The given control point must contain the date of the given data node (if parameter 'checkConsistence = true').
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
    * @pre    The given data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given data node is invalid (==nullptr).
    * @pre    The UID of the control point has to exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point does not exists for a control point instance (this can be checked via 'InstanceExists').
    * @pre    The given control point must contain the date of the given data node (if parameter 'checkConsistence = true').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    * @pre    The given control point must differ from the overwritten control point, but only either in the start point or in the end point.
    *         It is not possible to overwrite a single control point from a single date and simultaneously changing both ends of the time period.
    * @throw  SemanticRelationException, if the given control point does not differ from the overwritten control point or if the given control point differs in the start date and the end date from the overwritten control point.
    * @pre    The given control point must not overlap with an already existing control point.
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
    * @pre    The given data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given data node is invalid (==nullptr).
    * @pre    The UID of the control point has to exist for a control point instance.
    * @throw  SemanticRelationException, if the UID of the control point does not exists for a control point instance (this can be checked via 'InstanceExists').
    * @pre    The given control point must contain the date of the given data node (if parameter 'checkConsistence = true').
    * @throw  SemanticRelationException, if the given control point does not contain the date of the given data node and 'checkConsistence = true' (this can be checked via 'ControlPointManager::InsideControlPoint').
    *
    * @par dataNode         The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @par controlPoint     The control point instance to add link.
    * @par checkConsistence If true, the function checks, whether the date of the data node actually lies inside the control point to link.
    */
    void LinkDataToControlPoint(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence = true);
    /*
    * @brief  Unlink the given image from the linked control point.
    *         If data is unlinked from a control point, the function needs to check whether the control point is still linked to any other data:
    *           - if not, the control point instance will be removed (has to be removed since a control point with no associated data is not allowed).
    *           - if so, the function has to make sure that the control point instance is shortened to its minimum time period (e.g. moving the end point to an earlier date).
    *
    * @par dataNode       The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    */
    void UnlinkDataFromControlPoint(const DataNode* dataNode);
    /*
    * @brief  Set the information type of the given image.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given image data node is invalid (==nullptr).
    * @post   If the information type instance did not exist before, it is now added.
    *
    * @par imageNode        The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @par informationType  An information type that identifies the corresponding information type instance.
    */
    void AddInformationTypeToImage(const DataNode* imageNode, const SemanticTypes::InformationType& informationType);
    /*
    * @brief  Remove the information type of the given image.
    *         If the information type is removed, the function needs to check whether the information type is referenced by any other image:
    *           - if not, the information type instance can be removed (has to be removed since an information type with no associated image is not allowed).
    *           - if so, the information type is just removed from the given image.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  mitk::Exception if the given image data node is invalid (==nullptr).
    *
    * @par imageNode        The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    */
    void RemoveInformationTypeFromImage(const DataNode* imageNode);

  private:

    // the relation storage serves as a storage accessor and can be sub-classed for custom demands
    std::shared_ptr<RelationStorage> m_RelationStorage;
    DataStorage::Pointer m_DataStorage;
    /*
    * @brief A vector that stores the currently registered observer of this observable subject.
    */
    static std::vector<mitk::ISemanticRelationsObserver*> m_ObserverVector;
    /*
    * @brief The SemanticRelations, as an example of an observable subject, notifies (updates) the observer with a given case ID.
    *        The view's caseID was set before in the GUI. The parts of the view that observe changes in the semantic relations are only updated,
    *        if the given case ID is equal to the observer's current caseID and thus the observer currently shows the semantic information of the given case.
    *
    * @par  caseID    The caseID that identifies the currently active patient / case.
    */
    virtual void NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) const override;
    /*
    * @brief  Determine if the given control point contains images, which are connected to segmentations that represent the given lesion.
    *         If the lesion or the control point are not correctly stored, the function returns false.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par lesion         A Lesion with a UID that identifies the corresponding lesion instance.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    *
    * @return             True, if the given control point contains data that is related to the given lesion; false otherwise.
    */
    bool ControlPointContainsLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint) const;
    /*
    * @brief  Determine if the given control point contains images, which refer to the given information type.
    *         If the information type or the control point are not correctly stored, the function returns false.
    *
    * @par caseID           The current case identifier is defined by the given string.
    * @par informationType  An information type that identifies the corresponding information type instance.
    * @par controlPoint     A control point with a UID that identifies the corresponding control point instance.
    *
    * @return               True, if the given control point contains data that is related to the given information type; false otherwise.
    */
    bool ControlPointContainsInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ControlPoint& controlPoint) const;
    /*
    * @brief  Check if the given control point overlaps with an already existing control point.
    *         If the UID already exists, then the given control point may overlap with the previous or next control point,
    *         because the start point or the end point of the given control point might be modified (different to the same control point
    *         that is stored at the moment).
    *         If the UID does not already exists, the previous and next control point are found by comparing the dates of the already
    *         existing control points and the given control point.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    */
    bool CheckOverlappingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    /*
    * @brief  Check if the given control point is contained within an already existing control point.
    *         If the UID already exists, then the given control point is contained in this same control point.
    *         However, the function does not check if the given control point is really contained (just compares UID) in this case.
    *         If the UID does not already exist, the already existing control points are tested to see if they contain the
    *         given control point.
    *
    * @par caseID         The current case identifier is defined by the given string.
    * @par controlPoint   A control point with a UID that identifies the corresponding control point instance.
    */
    bool CheckContainingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
  };
} // namespace mitk

#endif // MITKSEMANTICRELATIONS_H
