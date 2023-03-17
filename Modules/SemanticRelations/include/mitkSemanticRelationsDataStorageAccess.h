/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSemanticRelationsDataStorageAccess_h
#define mitkSemanticRelationsDataStorageAccess_h

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

namespace mitk
{
  /**
  * @brief  The API provides functions to query and manipulate image relations and instances,
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
  class MITKSEMANTICRELATIONS_EXPORT SemanticRelationsDataStorageAccess
  {
  public:

    using DataNodeVector = std::vector<DataNode::Pointer>;

    SemanticRelationsDataStorageAccess(DataStorage* dataStorage);

    /************************************************************************/
    /* functions to get instances / attributes                              */
    /************************************************************************/
    /**
    * @brief  Return a vector of all segmentations that are currently available for the given case.
    *         The segmentations may be connected / not connected to a lesion of the case.
    *         If no segmentations are stored for the current case, an empty vector is returned.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the data storage member is invalid (==nullptr).
    *
    * @param caseID   The current case identifier is defined by the given string.
    *
    * @return         A vector of data nodes representing segmentations.
    */
    DataNodeVector GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID) const;
    /**
    * @brief  Return a vector of all segmentations that define the given lesion. These segmentations don't have to be linked to the same image.
    *         If the lesion is not referred to by any segmentation, an empty vector is returned.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the data storage member is invalid (==nullptr).
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   A lesion with a UID that identifies the corresponding lesion instance.
    *
    * @return         A vector of data nodes representing segmentations that define the given lesion.
    */
    DataNodeVector GetAllSegmentationsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;
    /**
    * @brief  Return a vector of all images that are currently available for the given case.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the data storage member is invalid (==nullptr).
    *
    * @param caseID   The current case identifier is defined by the given string.
    *
    * @return         A vector of data nodes representing images.
    */
    DataNodeVector GetAllImagesOfCase(const SemanticTypes::CaseID& caseID) const;
    /**
    * @brief  Return a vector of all images that are specified by the given vector of image IDs.
    *
    * @pre    The data storage member has to be valid (!nullptr).
    * @throw  SemanticRelationException, if the data storage member is invalid (==nullptr).
    *
    * @param imageIDs   A vector of image IDs that represent the images in the data storage.
    *
    * @return           A vector of data nodes representing images.
    */
    DataNodeVector GetAllImagesByID(const SemanticTypes::IDVector& imageIDs) const;
    /**
    * @brief Return a vector of all images that are connected to those segmentations that are linked to the given lesion.
    *         If the lesion is not referred to by any segmentation, an empty vector is returned.
    *
    * @pre    The UID of the lesion has to exist for a lesion instance.
    * @throw  SemanticRelationException, if UID of the lesion does not exist for a lesion instance (this can be checked via 'InstanceExists').
    *
    * @param caseID   The current case identifier is defined by the given string.
    * @param lesion   A lesion with a UID that identifies the corresponding lesion instance.
    *
    * @return         A vector of data nodes representing images on which the lesions are visible.
    */
    DataNodeVector GetAllImagesOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const;
    /**
    * @brief  Return a vector of all image nodes that are defined with the given control point and the given information type.
    *
    * @pre    The UID of the control point has to exist for a control point instance.
    *         The information type has to exist for the given case (and is therefore used by at least one data node).
    * @throw  SemanticRelationException, if the UID of the control point does not exist for a control point instance (this can be checked via 'InstanceExists') or
    *                                    if the information type is not used by any data node (this can be checked via 'InstanceExists').
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param controlPoint       A control point with a UID that identifies the corresponding control point instance.
    * @param informationType    An information type that identifies the corresponding information type instance.
    *
    * @return                   A vector of image nodes that are defined with the given control point and the given information type.
    */
    DataNodeVector GetAllSpecificImages(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const;
    /**
    * @brief  Return a vector of all image nodes that are defined with the given information type and the given examination period.
    *         The function uses the 'SemanticRelationsInference::GetAllImageIDsOfExaminationPeriod'-function to retrieve the imageIDs of the examination period and
    *         then compares the information type of all these images against the given information type.
    *
    * @param caseID
    * @param informationType      An information type that identifies the corresponding information type instance.
    * @param examinationPeriod    An examination period that identifies the corresponding examination period instance.
    *
    * @return                     A vector of image nodes that are defined with the given information type with the given control point.
    */
    DataNodeVector GetAllSpecificImages(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ExaminationPeriod& examinationPeriod) const;
    /**
    * @brief Return a vector of all segmentation nodes that are defined with the given control point and the given information type.
    *        The function uses the 'GetAllSpecificImages'-function to retrieve the specific images and then searches for the derived nodes (segmentation child nodes).
    *
    * @pre    The UID of the control point has to exist for a control point instance.
    *         The information type has to exist for the given case (and is therefore used by at least one data node).
    * @throw  SemanticRelationException, if the UID of the control point does not exist for a control point instance (this can be checked via 'InstanceExists') or
    *                                    if the information type is not used by any data node (this can be checked via 'InstanceExists').
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param controlPoint       A control point with a UID that identifies the corresponding control point instance.
    * @param informationType    An information type that identifies the corresponding information type instance.
    *
    * @return                   A vector of segmentation nodes that are defined with the given control point and the given information type.
    */
    DataNodeVector GetAllSpecificSegmentations(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const;
    /**
    * @brief Return the single segmentation node that is defined with the given information type, the given control point and is representing the given lesion.
    *        The function uses the 'GetAllSpecificSegmentations'-function to retrieve the specific segmentations and then checks for the represented lesion.
    *
    * @pre    The UID of the control point has to exist for a control point instance.
    *         The information type has to exist for the given case (and is therefore used by at least one data node).
    *         The lesion has to exist for the given case.
    * @throw  SemanticRelationException, if the UID of the control point does not exist for a control point instance (this can be checked via 'InstanceExists') or
    *                                    if the information type is not used by any data node (this can be checked via 'InstanceExists') or
    *                                    if the lesion does not exist for the given case (this can be checked via 'InstanceExists').
    *
    * @param caseID             The current case identifier is defined by the given string.
    * @param controlPoint       A control point with a UID that identifies the corresponding control point instance.
    * @param informationType    An information type that identifies the corresponding information type instance.
    * @param lesion             A lesion with a UID that identifies the corresponding lesion instance.
    *
    * @return                   A single segmentation node that is defined with the given information type, the given control point and is representing the given lesion.
    */
    DataNode::Pointer GetSpecificSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint,
                                              const SemanticTypes::InformationType& informationType, const SemanticTypes::Lesion& lesion) const;

  private:

    WeakPointer<DataStorage> m_DataStorage;

  };
} // namespace mitk

#endif
