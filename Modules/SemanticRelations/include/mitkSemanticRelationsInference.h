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

#ifndef MITKSEMANTICRELATIONSINFERENCE_H
#define MITKSEMANTICRELATIONSINFERENCE_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>

namespace mitk
{
  /**
  * @brief  The API provides functions to query image relations and instances
  *         that are helpful during follow-up examination, like control-points (time period),
  *         types of the images or lesions that may be visible on multiple images.
  *
  *   The class is able to generate IDs from given data nodes using DICOM information.
  *   These IDs are used to identify the corresponding instances of a specific case.
  *   The case can also be directly identified by the given case ID.
  *
  *   In order for most functions to work the case ID has to be used as a parameter.
  *   If not, these functions do nothing.
  */
  namespace SemanticRelationsInference
  {

    /************************************************************************/
    /* functions to get instances / attributes                              */
    /************************************************************************/
    /**
    * @brief Return a vector of lesion classes that are currently available for the given case.
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @return         A vector of lesion classes.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionClassVector GetAllLesionClassesOfCase(const SemanticTypes::CaseID& caseID);
    /**
    * @brief  Return the lesion that is defined by the given segmentation.
    *
    * @pre    The given segmentation data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given segmentation is invalid (==nullptr).
    * @pre    The segmentation data node has to represent a lesion. If not, the retrieved lesion will be empty, which leads to an exception.
    * @throw  SemanticRelationException, if the segmentation does not represent an existing lesion (this can be checked via 'IsRepresentingALesion').
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given data node.
    * @return                   The represented lesion.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::Lesion GetLesionOfSegmentation(const DataNode* segmentationNode);
    /**
    * @brief  Returns a vector of all lesions that are currently available for the current case and are connected to the given image (via a segmentation).
    *         If no lesions are stored for the current case, an empty vector is returned. If no segmentations are
    *         connected with the image node, no lesions for the specific image will be found and an empty vector is returned.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param imageNode    The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return             A vector of lesions.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionVector GetAllLesionsOfImage(const DataNode* imageNode);
    /**
    * @brief  Returns a vector of all lesions that are valid for the given case, given a specific control point.
    *
    * @param caseID         The current case identifier is defined by the given string.
    * @param controlPoint   A specific control point which has to be available at a returned (found) lesion:
    *                       Only those lesions are returned for which the image of the associated segmentation is linked to the given control point.
    *                       If the control point instance does not exist, an empty vector is returned.
    * @return               A vector of control points.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionVector GetAllLesionsOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    /**
    * @brief  Check if the given segmentation refers to an existing lesion instance.
    *         This function can be used before calling 'GetRepresentedLesion' in order to avoid a possible exception.
    *
    * @param segmentationNode   The segmentation identifier is extracted from the given data node.
    * @return                   True, if the segmentation refers to an existing lesion; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool IsRepresentingALesion(const DataNode* segmentationNode);
    /**
    * @brief  Check if the segmentation identified by the given segmentation ID refers to an existing lesion instance.
    *         This function can be used before calling 'GetRepresentedLesion' in order to avoid a possible exception.
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param segmentationID     The segmentation node identifier is defined by the given string.
    * @return                   True, if the segmentation refers to an existing lesion; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool IsRepresentingALesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);
    /**
    * @brief Check if the given lesion is present on the given data node.
    *        The function receives the case- and the node-ID from the DICOM tags of the node itself.
    *        It uses node predicates to decide if the node is an image or a segmentation node.
    *
    * @param lesion      A lesion with a UID that identifies the corresponding lesion instance.
    * @param dataNode    A data node to check.
    * @return            True, if the lesion is present on the data node; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool IsLesionPresent(const SemanticTypes::Lesion& lesion, const DataNode* dataNode);
    /**
    * @brief Check if the given lesion is related to the image identified by the given image ID.
    *        Each lesion is represented by a segmentation which is connected to its parent image.
    *
    * @param caseID       The current case identifier is defined by the given string.
    * @param lesion       A lesion with a UID that identifies the corresponding lesion instance.
    * @param imageID      The image node identifier is defined by the given string.
    * @return              True, if the lesion is related to image identified by the given image ID; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool IsLesionPresentOnImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ID& imageID);
    /**
    * @brief Check if the given lesion is present on the segmentation identified by the given segmentation ID.
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param lesion             A lesion with a UID that identifies the corresponding lesion instance.
    * @param segmentationID     The segmentation node identifier is defined by the given string.
    * @return                   True, if the lesion is present on the segmentation identified by the given segmentation ID; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool IsLesionPresentOnSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ID& segmentationID);
    /**
    * @brief Check if the given lesion is present at the given control point.
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param lesion             A lesion with a UID that identifies the corresponding lesion instance.
    * @param controlPoint       A control point with a UID that identifies the corresponding control point instance.
    * @return                   True, if the lesion is present at the given control point; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool IsLesionPresentAtControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint);
    /**
    * @brief Check if the given data node exists in the relation storage.
    *        The function receives the case- and the node-ID from the DICOM tags of the node itself.
    *        It uses node predicates to decide if the node is an image or a segmentation node and searches
    *        through the corresponding relations.
    *
    * @param dataNode    A data node to check.
    * @return            True, if the data node exists; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool InstanceExists(const DataNode* dataNode);
    /**
    * @brief  Check if the given lesion instance exists.
    *         This function can be used before calling 'AddLesionInstance' in order to avoid a possible exception.
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   A lesion with a UID that identifies the corresponding lesion instance.
    * @return         True, if the lesion instance exists; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Return a vector of all image IDs that identify images that are related to the given lesion.
    *         Each lesion is represented by a segmentation which is connected to its parent image.
    *         If the lesion is not represented by any segmentation, an empty vector is returned.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   A lesion with a UID that identifies the corresponding lesion instance.
    * @return         A vector of IDs identifying images that identify images that are related to the given lesion.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllImageIDsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /**
    * @brief  Return the control point of a data node.
    *         If the data node is not linked to a control point or the data node refers to a non-existing control point,
    *         a control point with an empty UID is returned.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param dataNode   The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return           The control point of the given data node.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GetControlPointOfImage(const DataNode* dataNode);
    /**
    * @brief Return a vector of all control points that are valid for the given case, given a specific lesion
    *
    * @param caseID           The current case identifier is defined by the given string.
    * @param lesion           A specific lesion which has to be available at a returned (found) control point:
    *                         Only those control points are returned for which an associated data has a segmentation that references the given lesion.
    *                         If the lesion does not exists, an empty vector is returned.
    * @return                 A vector of control points.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPointVector GetAllControlPointsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    /**
    * @brief Return a vector of all control points that are valid for the given case, given a specific information type.
    *
    * @param caseID           The current case identifier is defined by the given string.
    * @param informationType  A specific information type which has to be available at a returned (found) control point:
    *                         Only those control points are returned for which an associated data has the given information type.
    *                         If the information type instance does not exists, an empty vector is returned.
    * @return                 A vector of control points.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPointVector GetAllControlPointsOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType);
    /**
    * @brief  Check if the given control point instance exists.
    *         This function can be used before adding, linking and unlinking control points to avoid a possible exception.
    *
    * @param caseID         The current case identifier is defined by the given string.
    * @param controlPoint   A control point with a UID that identifies the corresponding control point instance.
    * @return               True, if the control point instance exists; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    /**
    * @brief  Check if the given examination period instance exists.
    *         This function can be used before calling 'AddExaminationPeriod' in order to avoid a possible exception.
    *
    * @param caseID               The current case identifier is defined by the given string.
    * @param examinationPeriod    An examination period with a UID that identifies the corresponding examination period instance.
    * @return                     True, if the examination period instance exists; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    /**
    * @brief  Return the information type of the given image.
    *         If the image does not contain any information type, an empty information type is returned.
    *
    * @pre    The given image data node has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the given image data node is invalid (==nullptr).
    *
    * @param imageNode    The current case identifier is extracted from the given data node, which contains DICOM information about the case.
    * @return             The information type of the given data node.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::InformationType GetInformationTypeOfImage(const DataNode* imageNode);
    /**
    * @brief  Return a vector of all information types that are valid for the given case, given a specific control point.
    *
    * @param caseID         The current case identifier is defined by the given string.
    * @param controlPoint   A specific control point which has to be available at a returned (found) information type:
    *                       Only those information types are returned for which an associated data is linked to the given control point.
    *                       If the control point instance does not exist, an empty vector is returned.
    * @return               A vector of information types.
    */
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::InformationTypeVector GetAllInformationTypesOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    /**
    * @brief  Check if the given information type exists.
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param informationType    An information type.
    * @return                   True, if the information type exists; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType);
    /**
    * @brief  Determine if the given information type contains images, which are connected to segmentations that represent the given lesion.
    *         If the lesion or the information type are not correctly stored, the function returns false.
    *
    * @param caseID           The current case identifier is defined by the given string.
    * @param lesion           A Lesion with a UID that identifies the corresponding lesion instance.
    * @param informationType  An information type that identifies the corresponding information type instance.
    *
    * @return                 True, if the given information type contains data that is related to the given lesion; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool SpecificImageExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::InformationType& informationType);
    /**
    * @brief  Determine if the given control point contains images, which are connected to segmentations that represent the given lesion.
    *         If the lesion or the control point are not correctly stored, the function returns false.
    *
    * @param caseID         The current case identifier is defined by the given string.
    * @param lesion         A Lesion with a UID that identifies the corresponding lesion instance.
    * @param controlPoint   A control point with a UID that identifies the corresponding control point instance.
    *
    * @return               True, if the given control point contains data that is related to the given lesion; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool SpecificImageExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint);
    /**
    * @brief  Determine if the given control point contains images, which refer to the given information type.
    *         If the information type or the control point are not correctly stored, the function returns false.
    *
    * @param caseID           The current case identifier is defined by the given string.
    * @param informationType  An information type that identifies the corresponding information type instance.
    * @param controlPoint     A control point with a UID that identifies the corresponding control point instance.
    *
    * @return                 True, if the given control point contains data that is related to the given information type; false otherwise.
    */
    MITKSEMANTICRELATIONS_EXPORT bool SpecificImageExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ControlPoint& controlPoint);

  } // namespace SemanticRelationsInference
} // namespace mitk

#endif // MITKSEMANTICRELATIONSINFERENCE_H
