/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSemanticRelationsDataStorageAccess.h"

// semantic relations module
#include "mitkControlPointManager.h"
#include "mitkDICOMHelper.h"
#include "mitkNodePredicates.h"
#include "mitkRelationStorage.h"
#include "mitkSemanticRelationException.h"
#include "mitkSemanticRelationsInference.h"

// c++
#include <iterator>
#include <algorithm>

mitk::SemanticRelationsDataStorageAccess::SemanticRelationsDataStorageAccess(DataStorage* dataStorage)
  : m_DataStorage(dataStorage)
{
  // nothing here
}

/************************************************************************/
/* functions to get instances / attributes                              */
/************************************************************************/

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID) const
{
  if (m_DataStorage.IsExpired())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }

  SemanticTypes::IDVector allSegmentationIDsOfCase = RelationStorage::GetAllSegmentationIDsOfCase(caseID);
  DataNodeVector allSegmentationsOfCase;
  // get all segmentation nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage.Lock()->GetSubset(NodePredicates::GetSegmentationPredicate());
  for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
  {
    DataNode* segmentationNode = it->Value();

    SemanticTypes::CaseID currentCaseID;
    SemanticTypes::ID segmentationID;
    try
    {
      // find the corresponding segmentation node for the given segmentation ID
      currentCaseID = GetCaseIDFromDataNode(segmentationNode);
      segmentationID = GetIDFromDataNode(segmentationNode);
    }
    catch (SemanticRelationException&)
    {
      // found a segmentation node that is not stored in the semantic relations
      // this segmentation node does not have any DICOM information --> exception thrown
      // continue with the next segmentation to compare IDs
      continue;
    }

    if (caseID == currentCaseID && (std::find(allSegmentationIDsOfCase.begin(), allSegmentationIDsOfCase.end(), segmentationID) != allSegmentationIDsOfCase.end()))
    {
      // found current image node in the storage, add it to the return vector
      allSegmentationsOfCase.push_back(segmentationNode);
    }
  }

  return allSegmentationsOfCase;
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllSegmentationsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  if (SemanticRelationsInference::InstanceExists(caseID, lesion))
  {
    // lesion exists, retrieve all case segmentations from the storage
    DataNodeVector allSegmentationsOfLesion = GetAllSegmentationsOfCase(caseID);

    // filter all segmentations: check for semantic relation with the given lesion using a lambda function
    auto lambda = [&lesion](DataNode::Pointer segmentation)
    {
      try
      {
        SemanticTypes::Lesion representedLesion = SemanticRelationsInference::GetLesionOfSegmentation(segmentation);
        return lesion.UID != representedLesion.UID;
      }
      catch (const SemanticRelationException&)
      {
        return true;
      }
    };
    allSegmentationsOfLesion.erase(std::remove_if(allSegmentationsOfLesion.begin(), allSegmentationsOfLesion.end(), lambda), allSegmentationsOfLesion.end());

    return allSegmentationsOfLesion;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing lesion instance for the given caseID " << caseID << " and lesion " << lesion.UID << ".";
  }
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllImagesOfCase(const SemanticTypes::CaseID& caseID) const
{
  if (m_DataStorage.IsExpired())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }

  SemanticTypes::IDVector allImageIDsOfCase = RelationStorage::GetAllImageIDsOfCase(caseID);
  DataNodeVector allImagesOfCase;
  // get all image nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer imageNodes = m_DataStorage.Lock()->GetSubset(NodePredicates::GetImagePredicate());
  for (auto it = imageNodes->Begin(); it != imageNodes->End(); ++it)
  {
    DataNode* imageNode = it->Value();

    SemanticTypes::CaseID currentCaseID;
    SemanticTypes::ID imageID;
    try
    {
      // find the corresponding image node for the given segmentation ID
      currentCaseID = GetCaseIDFromDataNode(imageNode);
      imageID = GetIDFromDataNode(imageNode);
    }
    catch (SemanticRelationException&)
    {
      // found an image node that is not stored in the semantic relations
      // this image node does not have any DICOM information --> exception thrown
      // continue with the next image to compare IDs
      continue;
    }

    if (caseID == currentCaseID && (std::find(allImageIDsOfCase.begin(), allImageIDsOfCase.end(), imageID) != allImageIDsOfCase.end()))
    {
      // found current image node in the storage, add it to the return vector
      allImagesOfCase.push_back(imageNode);
    }
  }

  return allImagesOfCase;
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllImagesByID(const SemanticTypes::IDVector& imageIDs) const
{
  if (m_DataStorage.IsExpired())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }

  DataNodeVector allImagesOfCase;
  // get all image nodes of the current data storage
  // only those nodes are respected, that are currently held in the data storage
  DataStorage::SetOfObjects::ConstPointer imageNodes = m_DataStorage.Lock()->GetSubset(NodePredicates::GetImagePredicate());
  for (auto it = imageNodes->Begin(); it != imageNodes->End(); ++it)
  {
    DataNode* imageNode = it->Value();

    SemanticTypes::CaseID currentCaseID;
    SemanticTypes::ID imageID;
    try
    {
      // find the corresponding image node for the given segmentation ID
      imageID = GetIDFromDataNode(imageNode);
    }
    catch (SemanticRelationException&)
    {
      // found an image node that is not stored in the semantic relations
      // this image node does not have any DICOM information --> exception thrown
      // continue with the next image to compare IDs
      continue;
    }

    if (std::find(imageIDs.begin(), imageIDs.end(), imageID) != imageIDs.end())
    {
      // found current image node in the storage, add it to the return vector
      allImagesOfCase.push_back(imageNode);
    }
  }

  return allImagesOfCase;
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllImagesOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  if (m_DataStorage.IsExpired())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }

  DataNodeVector allImagesOfLesion;
  // 1. get all segmentations that define the lesion
  // 2. retrieve the parent node (source) of the found segmentation node
  DataNodeVector allSegmentationsOfLesion = GetAllSegmentationsOfLesion(caseID, lesion);
  for (const auto& segmentationNode : allSegmentationsOfLesion)
  {
    // get parent node of the current segmentation node with the node predicate
    DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage.Lock()->GetSources(segmentationNode, NodePredicates::GetImagePredicate(), false);
    for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
    {
      DataNode::Pointer dataNode = it->Value();
      allImagesOfLesion.push_back(it->Value());
    }
  }

  std::sort(allImagesOfLesion.begin(), allImagesOfLesion.end());
  allImagesOfLesion.erase(std::unique(allImagesOfLesion.begin(), allImagesOfLesion.end()), allImagesOfLesion.end());
  return allImagesOfLesion;
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllSpecificImages(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const
{
  if (SemanticRelationsInference::InstanceExists(caseID, controlPoint))
  {
    if (SemanticRelationsInference::InstanceExists(caseID, informationType))
    {
      // control point exists, information type exists, retrieve all images from the storage
      DataNodeVector allImagesOfCase = GetAllImagesOfCase(caseID);
      // filter all images to remove the ones with a different control point and information type using a lambda function
      auto lambda = [&controlPoint, &informationType](DataNode::Pointer imageNode)
      {
        return (informationType != SemanticRelationsInference::GetInformationTypeOfImage(imageNode))
            || (controlPoint.date != SemanticRelationsInference::GetControlPointOfImage(imageNode).date);
      };

      allImagesOfCase.erase(std::remove_if(allImagesOfCase.begin(), allImagesOfCase.end(), lambda), allImagesOfCase.end());

      return allImagesOfCase;
    }
    else
    {
      mitkThrowException(SemanticRelationException) << "Could not find an existing information type for the given caseID " << caseID << " and information type " << informationType << ".";
    }
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing control point for the given caseID " << caseID << " and control point " << controlPoint.UID << ".";
  }
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllSpecificImages(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ExaminationPeriod& examinationPeriod) const
{
  if (SemanticRelationsInference::InstanceExists(caseID, informationType))
  {
    if (SemanticRelationsInference::InstanceExists(caseID, examinationPeriod))
    {
      // examination period exists, information type exists, retrieve all imageIDs from the storage
      auto allImageIDsOfExaminationPeriod = SemanticRelationsInference::GetAllImageIDsOfExaminationPeriod(caseID, examinationPeriod);
      // filter all images to remove the ones with a different information type using a lambda function
      auto lambda = [&caseID, &informationType](SemanticTypes::ID imageID)
      {
        return (informationType != RelationStorage::GetInformationTypeOfImage(caseID, imageID));
      };

      allImageIDsOfExaminationPeriod.erase(std::remove_if(allImageIDsOfExaminationPeriod.begin(), allImageIDsOfExaminationPeriod.end(), lambda), allImageIDsOfExaminationPeriod.end());

      auto allImagesOfExaminationPeriod = GetAllImagesByID(allImageIDsOfExaminationPeriod);
      return allImagesOfExaminationPeriod;
    }
    else
    {
      mitkThrowException(SemanticRelationException) << "Could not find an existing examination period for the given caseID " << caseID << " and examination period " << examinationPeriod.name << ".";
    }
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing information type for the given caseID " << caseID << " and information type " << informationType << ".";
  }
}

mitk::SemanticRelationsDataStorageAccess::DataNodeVector mitk::SemanticRelationsDataStorageAccess::GetAllSpecificSegmentations(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const
{
  if (m_DataStorage.IsExpired())
  {
    mitkThrow() << "Not a valid data storage.";
  }

  DataNodeVector allSpecificImages;
  try
  {
    allSpecificImages = GetAllSpecificImages(caseID, controlPoint, informationType);

  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get the specific segmentation.";
  }

  DataNodeVector allSpecificSegmentations;
  for (const auto& imageNode : allSpecificImages)
  {
    DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage.Lock()->GetDerivations(imageNode, NodePredicates::GetSegmentationPredicate(), false);
    for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
    {
      allSpecificSegmentations.push_back(it->Value());
    }
  }

  return allSpecificSegmentations;
}

mitk::DataNode::Pointer mitk::SemanticRelationsDataStorageAccess::GetSpecificSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint,
  const SemanticTypes::InformationType& informationType, const SemanticTypes::Lesion& lesion) const
{
  if (m_DataStorage.IsExpired())
  {
    mitkThrow() << "Not a valid data storage.";
  }

  DataNodeVector allSpecificSegmentations;
  try
  {
    allSpecificSegmentations = GetAllSpecificSegmentations(caseID, controlPoint, informationType);

  }
  catch (SemanticRelationException& e)
  {
      mitkReThrow(e) << "Cannot get the specific segmentation.";
  }

  for (const auto& segmentationNode : allSpecificSegmentations)
  {
    SemanticTypes::Lesion representedLesion = SemanticRelationsInference::GetLesionOfSegmentation(segmentationNode);
    if (representedLesion.UID == lesion.UID)
    {
      return segmentationNode;
    }
  }

  return mitk::DataNode::Pointer();
}
