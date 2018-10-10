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

// semantic relations module
#include "mitkControlPointManager.h"
#include "mitkNodePredicates.h"
#include "mitkSemanticRelations.h"
#include "mitkSemanticRelationException.h"
#include "mitkUIDGeneratorBoost.h"

// multi label module
#include <mitkLabelSetImage.h>

// c++
#include <iterator>
#include <algorithm>

std::vector<mitk::ISemanticRelationsObserver*> mitk::SemanticRelations::m_ObserverVector;

mitk::SemanticRelations::SemanticRelations(DataStorage::Pointer dataStorage)
  : m_DataStorage(dataStorage)
{
  m_RelationStorage = std::make_shared<RelationStorage>();
  m_RelationStorage->SetDataStorage(m_DataStorage);
}

mitk::SemanticRelations::~SemanticRelations()
{
  // nothing here
}

void mitk::SemanticRelations::AddObserver(ISemanticRelationsObserver* observer)
{
  std::vector<ISemanticRelationsObserver*>::iterator existingObserver = std::find(m_ObserverVector.begin(), m_ObserverVector.end(), observer);
  if (existingObserver != m_ObserverVector.end())
  {
    // no need to add the already existing observer
    return;
  }

  m_ObserverVector.push_back(observer);
}

void mitk::SemanticRelations::RemoveObserver(ISemanticRelationsObserver* observer)
{
  m_ObserverVector.erase(std::remove(m_ObserverVector.begin(), m_ObserverVector.end(), observer), m_ObserverVector.end());
}

/************************************************************************/
/* functions to get instances / attributes                              */
/************************************************************************/

mitk::SemanticTypes::LesionVector mitk::SemanticRelations::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllLesionsOfCase(caseID);
}

mitk::SemanticTypes::LesionVector mitk::SemanticRelations::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  SemanticTypes::LesionVector allLesions = GetAllLesionsOfCase(caseID);

  // filter the lesions: use only those, where the associated data is connected to image data that refers to the given control point using a lambda function
  auto lambda = [&caseID, &controlPoint, this](const SemanticTypes::Lesion& lesion) { return !ControlPointContainsLesion(caseID, lesion, controlPoint); };
  allLesions.erase(std::remove_if(allLesions.begin(), allLesions.end(), lambda), allLesions.end());

  return allLesions;
}

mitk::SemanticTypes::LesionClassVector mitk::SemanticRelations::GetAllLesionClassesOfCase(const SemanticTypes::CaseID& caseID) const
{
  SemanticTypes::LesionVector allLesionsOfCase = GetAllLesionsOfCase(caseID);
  SemanticTypes::LesionClassVector allLesionClassesOfCase;

  for (const auto& lesion : allLesionsOfCase)
  {
    allLesionClassesOfCase.push_back(lesion.lesionClass);
  }

  // remove duplicate entries
  auto lessThan = [](const SemanticTypes::LesionClass& lesionClassL, const SemanticTypes::LesionClass& lesionClassR)
  {
    return lesionClassL.UID < lesionClassR.UID;
  };

  auto equal = [](const SemanticTypes::LesionClass& lesionClassL, const SemanticTypes::LesionClass& lesionClassR)
  {
    return lesionClassL.UID == lesionClassR.UID;
  };

  std::sort(allLesionClassesOfCase.begin(), allLesionClassesOfCase.end(), lessThan);
  allLesionClassesOfCase.erase(std::unique(allLesionClassesOfCase.begin(), allLesionClassesOfCase.end(), equal), allLesionClassesOfCase.end());

  return allLesionClassesOfCase;
}

mitk::SemanticTypes::LesionVector mitk::SemanticRelations::GetAllLesionsInImage(const DataNode* imageNode) const
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  if (m_DataStorage.IsNull())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }

  SemanticTypes::LesionVector allLesionsInImage;
  // get child nodes of the current node with the segmentation predicate
  DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage->GetDerivations(imageNode, NodePredicates::GetSegmentationPredicate(), false);
  for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
  {
    DataNode* segmentationNode = it->Value();
    try
    {
      SemanticTypes::Lesion representedLesion = GetRepresentedLesion(segmentationNode);
      allLesionsInImage.push_back(representedLesion);
    }
    catch (const SemanticRelationException&)
    {
      continue;
    }
  }
  return allLesionsInImage;
}

mitk::SemanticTypes::Lesion mitk::SemanticRelations::GetRepresentedLesion(const DataNode* segmentationNode) const
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationID = GetIDFromDataNode(segmentationNode);
  SemanticTypes::Lesion representedLesion = m_RelationStorage->GetRepresentedLesion(caseID, segmentationID);

  if (representedLesion.UID.empty())
  {
    mitkThrowException(SemanticRelationException) << "Could not find a represented lesion instance for the given segmentation node " << segmentationNode->GetName();
  }
  else
  {
    return representedLesion;
  }
}

bool mitk::SemanticRelations::IsRepresentingALesion(const DataNode* segmentationNode) const
{
  try
  {
    SemanticTypes::Lesion representedLesion = GetRepresentedLesion(segmentationNode);
    return true;
  }
  catch (const Exception&)
  {
    return false;
  }
}

bool mitk::SemanticRelations::InstanceExists(const DataNode* dataNode) const
{
  try
  {
    SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
    SemanticTypes::ID dataNodeID = GetIDFromDataNode(dataNode);

    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      std::vector<std::string> allImageIDsOfCase = m_RelationStorage->GetAllImageIDsOfCase(caseID);
      return std::find(allImageIDsOfCase.begin(), allImageIDsOfCase.end(), dataNodeID) != allImageIDsOfCase.end();
    }
    else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      std::vector<std::string> allSegmentationIDsOfCase = m_RelationStorage->GetAllSegmentationIDsOfCase(caseID);
      return std::find(allSegmentationIDsOfCase.begin(), allSegmentationIDsOfCase.end(), dataNodeID) != allSegmentationIDsOfCase.end();
    }
    else
    {
      return false;
    }
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }
  return m_RelationStorage->GetAllSegmentationsOfCase(caseID);
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllSegmentationsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }

  if (InstanceExists(caseID, lesion))
  {
    // lesion exists, retrieve all case segmentations from the storage
    DataNodeVector allSegmentationsOfLesion = GetAllSegmentationsOfCase(caseID);

    // filter all segmentations: check for semantic relation with the given lesion using a lambda function
    auto lambda = [&lesion, this](DataNode::Pointer segmentation)
    {
      try
      {
        SemanticTypes::Lesion representedLesion = GetRepresentedLesion(segmentation);
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

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllImagesOfCase(const SemanticTypes::CaseID& caseID) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data storage.";
  }
  return m_RelationStorage->GetAllImagesOfCase(caseID);
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllImagesOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  if (m_DataStorage.IsNull())
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
    DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage->GetSources(segmentationNode, NodePredicates::GetImagePredicate(), false);
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

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  mitk::SemanticTypes::LesionVector allLesions = GetAllLesionsOfCase(caseID);

  // filter all lesions: check for equality with the given lesion using a lambda function
  auto lambda = [&lesion](const SemanticTypes::Lesion& currentLesion) { return currentLesion.UID == lesion.UID; };
  const auto existingLesion = std::find_if(allLesions.begin(), allLesions.end(), lambda);

  if (existingLesion != allLesions.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

mitk::SemanticTypes::ControlPointVector mitk::SemanticRelations::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllControlPointsOfCase(caseID);
}

mitk::SemanticTypes::ControlPointVector mitk::SemanticRelations::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  SemanticTypes::ControlPointVector allControlPoints = GetAllControlPointsOfCase(caseID);

  // filter the control points: use only those, where the associated image data has a segmentation that refers to the given lesion using a lambda function
  auto lambda = [&caseID, &lesion, this](const SemanticTypes::ControlPoint& controlPoint) { return !ControlPointContainsLesion(caseID, lesion, controlPoint); };
  allControlPoints.erase(std::remove_if(allControlPoints.begin(), allControlPoints.end(), lambda), allControlPoints.end());

  return allControlPoints;
}

mitk::SemanticTypes::ControlPointVector mitk::SemanticRelations::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const
{
  SemanticTypes::ControlPointVector allControlPoints = GetAllControlPointsOfCase(caseID);

  // filter the control points: use only those, where the associated image data refers to the given information type using a lambda function
  auto lambda = [&caseID, &informationType, this](const SemanticTypes::ControlPoint& controlPoint) { return !ControlPointContainsInformationType(caseID, informationType, controlPoint); };
  allControlPoints.erase(std::remove_if(allControlPoints.begin(), allControlPoints.end(), lambda), allControlPoints.end());

  return allControlPoints;
}

mitk::SemanticTypes::ControlPoint mitk::SemanticRelations::GetControlPointOfData(const DataNode* imageNode) const
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID dataNodeID = GetIDFromDataNode(imageNode);
  return m_RelationStorage->GetControlPointOfImage(caseID, dataNodeID);
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllImagesOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "Not a valid data storage.";
  }

  if (InstanceExists(caseID, controlPoint))
  {
    // control point exists, retrieve all images from the storage
    DataNodeVector allImagesOfControlPoint = GetAllImagesOfCase(caseID);

    // filter all images to remove the ones with a different control point using a lambda function
    auto lambda = [&controlPoint, this](DataNode::Pointer imageNode)
    {
      return controlPoint.UID != GetControlPointOfData(imageNode).UID;
    };

    allImagesOfControlPoint.erase(std::remove_if(allImagesOfControlPoint.begin(), allImagesOfControlPoint.end(), lambda), allImagesOfControlPoint.end());

    return allImagesOfControlPoint;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing control point instance for the given caseID " << caseID << " and control point " << controlPoint.UID << ".";
  }
}

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  SemanticTypes::ControlPointVector allControlPoints = GetAllControlPointsOfCase(caseID);

  // filter all control points: check for equality with the given control point using a lambda function
  auto lambda = [&controlPoint](const SemanticTypes::ControlPoint& currentControlPoint) { return currentControlPoint.UID == controlPoint.UID; };
  const auto existingControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(), lambda);

  if (existingControlPoint != allControlPoints.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

mitk::SemanticTypes::ExaminationPeriodVector mitk::SemanticRelations::GetAllExaminationPeriodsOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllExaminationPeriodsOfCase(caseID);
}

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod) const
{
  SemanticTypes::ExaminationPeriodVector allExaminationPeriods = GetAllExaminationPeriodsOfCase(caseID);

  // filter all examination periods: check for equality with the given examination period using a lambda function
  auto lambda = [&examinationPeriod](const SemanticTypes::ExaminationPeriod& currentExaminationPeriod) { return currentExaminationPeriod.UID == examinationPeriod.UID; };
  const auto existingExaminationPeriod = std::find_if(allExaminationPeriods.begin(), allExaminationPeriods.end(), lambda);

  if (existingExaminationPeriod != allExaminationPeriods.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

mitk::SemanticTypes::InformationTypeVector mitk::SemanticRelations::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllInformationTypesOfCase(caseID);
}

mitk::SemanticTypes::InformationTypeVector mitk::SemanticRelations::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  SemanticTypes::InformationTypeVector allInformationTypes = GetAllInformationTypesOfCase(caseID);

  // filter the information types: use only those, where the associated data refers to the given control point using a lambda function
  auto lambda = [&caseID, &controlPoint, this](const SemanticTypes::InformationType& informationType) { return !ControlPointContainsInformationType(caseID, informationType, controlPoint); };
  allInformationTypes.erase(std::remove_if(allInformationTypes.begin(), allInformationTypes.end(), lambda), allInformationTypes.end());

  return allInformationTypes;
}

mitk::SemanticTypes::InformationType mitk::SemanticRelations::GetInformationTypeOfImage(const DataNode* imageNode) const
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID imageID = GetIDFromDataNode(imageNode);
  return m_RelationStorage->GetInformationTypeOfImage(caseID, imageID);
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllImagesOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "Not a valid data storage.";
  }

  if (InstanceExists(caseID, informationType))
  {
    // information type exists, retrieve all images from the storage
    DataNodeVector allImagesOfInformationType = GetAllImagesOfCase(caseID);

    // filter all images to remove the ones with a different information type using a lambda function
    auto lambda = [&informationType, this](DataNode::Pointer imageNode)
    {
      return informationType != GetInformationTypeOfImage(imageNode);
    };

    allImagesOfInformationType.erase(std::remove_if(allImagesOfInformationType.begin(), allImagesOfInformationType.end(), lambda), allImagesOfInformationType.end());

    return allImagesOfInformationType;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing information type for the given caseID " << caseID << " and information type " << informationType;
  }
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllSpecificImages(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "Not a valid data storage.";
  }

  if (InstanceExists(caseID, controlPoint))
  {
    if (InstanceExists(caseID, informationType))
    {
      // control point exists, information type exists, retrieve all images from the storage
      DataNodeVector allImagesOfCase = GetAllImagesOfCase(caseID);
      // filter all images to remove the ones with a different control point and information type using a lambda function
      auto lambda = [&controlPoint, &informationType, this](DataNode::Pointer imageNode)
      {
        return (informationType != GetInformationTypeOfImage(imageNode)) || (controlPoint != GetControlPointOfData(imageNode));
      };

      allImagesOfCase.erase(std::remove_if(allImagesOfCase.begin(), allImagesOfCase.end(), lambda), allImagesOfCase.end());

      return allImagesOfCase;
    }
    else
    {
      mitkThrowException(SemanticRelationException) << "Could not find an existing information type for the given caseID " << caseID << " and information type " << informationType;
    }
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing control point for the given caseID " << caseID << " and control point " << controlPoint.UID;
  }
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllSpecificSegmentations(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const
{
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "Not a valid data storage.";
  }

  DataNodeVector allSpecificImages = GetAllSpecificImages(caseID, controlPoint, informationType);
  DataNodeVector allSpecificSegmentations;
  for (const auto& imageNode : allSpecificImages)
  {
    DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage->GetDerivations(imageNode, NodePredicates::GetSegmentationPredicate(), false);
    for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
    {
      allSpecificSegmentations.push_back(it->Value());
    }
  }

  return allSpecificSegmentations;
}

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const
{
  SemanticTypes::InformationTypeVector allInformationTypes = GetAllInformationTypesOfCase(caseID);

  // filter all information types: check for equality with the given information type using a lambda function
  auto lambda = [&informationType](const SemanticTypes::InformationType& currentInformationType) { return currentInformationType == informationType; };
  const auto existingInformationType = std::find_if(allInformationTypes.begin(), allInformationTypes.end(), lambda);

  if (existingInformationType != allInformationTypes.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

std::vector<mitk::SemanticTypes::CaseID> mitk::SemanticRelations::GetAllCaseIDs() const
{
  return m_RelationStorage->GetAllCaseIDs();
}

/************************************************************************/
/* functions to add / remove instances / attributes                     */
/************************************************************************/

void mitk::SemanticRelations::AddImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID nodeID = GetIDFromDataNode(imageNode);

  m_RelationStorage->AddCase(caseID);
  m_RelationStorage->AddImage(caseID, nodeID);

  SemanticTypes::InformationType informationType = GetDICOMModalityFromDataNode(imageNode);
  AddInformationTypeToImage(imageNode, informationType);

  // set the correct control point for this image
  SemanticTypes::ControlPoint controlPoint = GenerateControlPoint(imageNode);
  SetControlPointOfData(imageNode, controlPoint);
}

void mitk::SemanticRelations::RemoveImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID nodeID = GetIDFromDataNode(imageNode);

  RemoveInformationTypeFromImage(imageNode);
  UnlinkDataFromControlPoint(imageNode);
  m_RelationStorage->RemoveImage(caseID, nodeID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (InstanceExists(caseID, lesion))
  {
    mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to add already exists for the given case.";
  }
  else
  {
    m_RelationStorage->AddLesion(caseID, lesion);
    NotifyObserver(caseID);
  }
}

void mitk::SemanticRelations::OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (InstanceExists(caseID, lesion))
  {
    m_RelationStorage->OverwriteLesion(caseID, lesion);
    NotifyObserver(caseID);
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to overwrite does not exist for the given case.";
  }
}

void mitk::SemanticRelations::AddLesionAndLinkSegmentation(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  AddLesion(caseID, lesion);
  LinkSegmentationToLesion(segmentationNode, lesion);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (InstanceExists(caseID, lesion))
  {
    DataNodeVector allSegmentationsOfLesion = GetAllSegmentationsOfLesion(caseID, lesion);
    if (allSegmentationsOfLesion.empty())
    {
      // no more segmentations are linked to the specific lesion
      // the lesion can be removed from the storage
      m_RelationStorage->RemoveLesion(caseID, lesion);
      NotifyObserver(caseID);
    }
    else
    {
      mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to remove is still referred to by a segmentation node. Lesion will not be removed.";
    }
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to remove does not exist for the given case.";
  }
}

void mitk::SemanticRelations::AddSegmentation(const DataNode* segmentationNode, const DataNode* parentNode)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  if (nullptr == parentNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid parent data node.";
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationNodeID = GetIDFromDataNode(segmentationNode);
  SemanticTypes::ID parentNodeID = GetIDFromDataNode(parentNode);

  m_RelationStorage->AddSegmentation(caseID, segmentationNodeID, parentNodeID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::LinkSegmentationToLesion(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  if (InstanceExists(caseID, lesion))
  {
    SemanticTypes::ID segmentationID = GetIDFromDataNode(segmentationNode);
    m_RelationStorage->LinkSegmentationToLesion(caseID, segmentationID, lesion);
    NotifyObserver(caseID);
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to link does not exist for the given case.";
  }
}

void mitk::SemanticRelations::UnlinkSegmentationFromLesion(const DataNode* segmentationNode)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationID = GetIDFromDataNode(segmentationNode);
  m_RelationStorage->UnlinkSegmentationFromLesion(caseID, segmentationID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::RemoveSegmentation(const DataNode* segmentationNode)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  // continue with a valid data node
  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationNodeID = GetIDFromDataNode(segmentationNode);
  m_RelationStorage->RemoveSegmentation(caseID, segmentationNodeID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::SetControlPointOfData(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  SemanticTypes::ControlPointVector allControlPoints = GetAllControlPointsOfCase(caseID);
  // need to check if an already existing control point fits/contains the user control point
  SemanticTypes::ControlPoint existingControlPoint = FindExistingControlPoint(controlPoint, allControlPoints);
  if (!existingControlPoint.UID.empty())
  {
    try
    {
      // found an already existing control point
      LinkDataToControlPoint(dataNode, existingControlPoint, false);
    }
    catch (const SemanticRelationException&)
    {
      mitkThrowException(SemanticRelationException) << "The data can not be linked. Inconsistency in the semantic relations storage assumed.";
    }
  }
  else
  {
    try
    {
      AddControlPointAndLinkData(dataNode, controlPoint, false);
      // added a new control point
      // find closest control point to add the new control point to the correct examination period
      SemanticTypes::ControlPoint closestControlPoint = FindClosestControlPoint(controlPoint, allControlPoints);
      SemanticTypes::ExaminationPeriodVector allExaminationPeriods = GetAllExaminationPeriodsOfCase(caseID);
      SemanticTypes::ExaminationPeriod examinationPeriod = FindExaminationPeriod(closestControlPoint, allExaminationPeriods);
      if (examinationPeriod.UID.empty())
      {
        // no closest control point (exceed threshold) or no examination period found
        // create a new examination period for this control point and add it to the storage
        examinationPeriod.UID = UIDGeneratorBoost::GenerateUID();
        AddExaminationPeriod(caseID, examinationPeriod);
      }

      // add the control point to the (newly created or found / close) examination period
      AddControlPointToExaminationPeriod(caseID, controlPoint, examinationPeriod);
    }
    catch (const SemanticRelationException&)
    {
      mitkThrowException(SemanticRelationException) << "The data can not be linked. Inconsistency in the semantic relations storage assumed.";
    }
  }

  ClearControlPoints(caseID);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::AddControlPointAndLinkData(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  if (InstanceExists(caseID, controlPoint))
  {
    mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to add already exists for the given case. \n Use 'LinkDataToControlPoint' instead.";
  }

  m_RelationStorage->AddControlPoint(caseID, controlPoint);
  LinkDataToControlPoint(dataNode, controlPoint, checkConsistence);
}

void mitk::SemanticRelations::LinkDataToControlPoint(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  if (InstanceExists(caseID, controlPoint))
  {
    SemanticTypes::ID dataID = GetIDFromDataNode(dataNode);
    m_RelationStorage->LinkDataToControlPoint(caseID, dataID, controlPoint);
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to link does not exist for the given case.";
  }
}

void mitk::SemanticRelations::UnlinkDataFromControlPoint(const DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  SemanticTypes::ID dataID = GetIDFromDataNode(dataNode);
  SemanticTypes::ControlPoint controlPoint = m_RelationStorage->GetControlPointOfImage(caseID, dataID);
  m_RelationStorage->UnlinkDataFromControlPoint(caseID, dataID);
  ClearControlPoints(caseID);
}

void mitk::SemanticRelations::AddExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  if (InstanceExists(caseID, examinationPeriod))
  {
    mitkThrowException(SemanticRelationException) << "The examination period " << examinationPeriod.UID << " to add already exists for the given case.";
  }
  else
  {
    m_RelationStorage->AddExaminationPeriod(caseID, examinationPeriod);
  }
}

void mitk::SemanticRelations::AddControlPointToExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  if (!InstanceExists(caseID, controlPoint))
  {
    mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to add does not exist for the given case.";
  }

  if (!InstanceExists(caseID, examinationPeriod))
  {
    mitkThrowException(SemanticRelationException) << "The examination period " << examinationPeriod.UID << " does not exist for the given case. \n Use 'AddExaminationPeriod' before.";
  }

  m_RelationStorage->AddControlPointToExaminationPeriod(caseID, controlPoint, examinationPeriod);
}

void mitk::SemanticRelations::SetInformationType(const DataNode* imageNode, const SemanticTypes::InformationType& informationType)
{
  RemoveInformationTypeFromImage(imageNode);
  AddInformationTypeToImage(imageNode, informationType);

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  NotifyObserver(caseID);
}

void mitk::SemanticRelations::AddInformationTypeToImage(const DataNode* imageNode, const SemanticTypes::InformationType& informationType)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID imageID = GetIDFromDataNode(imageNode);
  m_RelationStorage->AddInformationTypeToImage(caseID, imageID, informationType);
}

void mitk::SemanticRelations::RemoveInformationTypeFromImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID imageID = GetIDFromDataNode(imageNode);
  SemanticTypes::InformationType originalInformationType = m_RelationStorage->GetInformationTypeOfImage(caseID, imageID);
  m_RelationStorage->RemoveInformationTypeFromImage(caseID, imageID);

  // check for further references to the removed information type
  std::vector<std::string> allImageIDsVectorValue = m_RelationStorage->GetAllImageIDsOfCase(caseID);
  for (const auto otherImageID : allImageIDsVectorValue)
  {
    SemanticTypes::InformationType otherInformationType = m_RelationStorage->GetInformationTypeOfImage(caseID, otherImageID);
    if (otherInformationType == originalInformationType)
    {
      // found the information type in another image -> cannot remove the information type from the case
      return;
    }
  }

  // given information type was not referred by any other image of the case -> the information type can be removed from the case
  m_RelationStorage->RemoveInformationTypeFromCase(caseID, originalInformationType);
}

/************************************************************************/
/* private functions                                                    */
/************************************************************************/
void mitk::SemanticRelations::NotifyObserver(const SemanticTypes::CaseID& caseID) const
{
  for (auto& observer : m_ObserverVector)
  {
    observer->Update(caseID);
  }
}

bool mitk::SemanticRelations::ControlPointContainsLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint) const
{
  DataNodeVector allImagesOfLesion;
  try
  {
    allImagesOfLesion = GetAllImagesOfLesion(caseID, lesion);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving image data; lesion has to be outside the control point
    return false;
  }

  DataNodeVector allImagesOfControlPoint;
  try
  {
    allImagesOfControlPoint = GetAllImagesOfControlPoint(caseID, controlPoint);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving control point data; lesion has to be outside the control point
    return false;
  }

  std::sort(allImagesOfLesion.begin(), allImagesOfLesion.end());
  std::sort(allImagesOfControlPoint.begin(), allImagesOfControlPoint.end());
  DataNodeVector allImagesIntersection;
  // set intersection removes duplicated nodes, since 'GetAllImagesOfControlPoint' only contains at most one of each node
  std::set_intersection(allImagesOfLesion.begin(), allImagesOfLesion.end(),
                        allImagesOfControlPoint.begin(), allImagesOfControlPoint.end(),
                        std::back_inserter(allImagesIntersection));

  // if the vector of intersecting data is empty, the control point does not contain the lesion
  return !allImagesIntersection.empty();
}

bool mitk::SemanticRelations::ControlPointContainsInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ControlPoint& controlPoint) const
{
  DataNodeVector allImagesIntersection = GetAllSpecificImages(caseID, controlPoint, informationType);
  return !allImagesIntersection.empty();
}

void mitk::SemanticRelations::ClearControlPoints(const SemanticTypes::CaseID& caseID)
{
  SemanticTypes::ControlPointVector allControlPointsOfCase;
  try
  {
    allControlPointsOfCase = GetAllControlPointsOfCase(caseID);

  }
  catch (const SemanticRelationException&)
  {
    // error retrieving control point data
    return;
  }

  DataNodeVector allImagesOfCase;
  try
  {
    allImagesOfCase = GetAllImagesOfCase(caseID);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving image data
    return;
  }

  SemanticTypes::ControlPointVector allControlPoints;
  for (const auto& image : allImagesOfCase)
  {
    try
    {
      allControlPoints.push_back(GetControlPointOfData(image));

    }
    catch (const SemanticRelationException&)
    {
      // error retrieving control point of data
      continue;
    }
  }

  std::sort(allControlPointsOfCase.begin(), allControlPointsOfCase.end());
  std::sort(allControlPoints.begin(), allControlPoints.end());
  SemanticTypes::ControlPointVector allControlPointsDifference;

  std::set_difference(allControlPointsOfCase.begin(), allControlPointsOfCase.end(),
                      allControlPoints.begin(), allControlPoints.end(),
                      std::inserter(allControlPointsDifference, allControlPointsDifference.begin()));

  auto allExaminationPeriods = GetAllExaminationPeriodsOfCase(caseID);
  for (const auto& controlPoint : allControlPointsDifference)
  {
    const auto& examinationPeriod = FindExaminationPeriod(controlPoint, allExaminationPeriods);
    m_RelationStorage->RemoveControlPointFromExaminationPeriod(caseID, controlPoint, examinationPeriod);
    m_RelationStorage->RemoveControlPointFromCase(caseID, controlPoint);
  }
}
