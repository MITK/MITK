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
#include "mitkSemanticRelations.h"
#include "mitkSemanticRelationException.h"
#include "mitkNodePredicates.h"
#include "mitkUIDGeneratorBoost.h"

// multi label module
#include <mitkLabelSetImage.h>

// c++
#include <algorithm>

mitk::SemanticRelations::SemanticRelations(mitk::DataStorage::Pointer dataStorage)
  : m_DataStorage(dataStorage)
{
  m_RelationStorage = std::make_shared<mitk::RelationStorage>();
  m_RelationStorage->SetDataStorage(m_DataStorage);
}

mitk::SemanticRelations::~SemanticRelations()
{
  // nothing here
}

mitk::RelationStorage& mitk::SemanticRelations::GetRelationStorage()
{
  return *m_RelationStorage;
}

/************************************************************************/
/* functions to get instances / attributes                              */
/************************************************************************/

mitk::SemanticRelations::LesionVector mitk::SemanticRelations::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllLesionsOfCase(caseID);
}

mitk::SemanticRelations::LesionVector mitk::SemanticRelations::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  LesionVector allLesions = GetAllLesionsOfCase(caseID);

  // filter the lesions: use only those, where the associated data is connected to image data that refers to the given control point using a lambda function
  auto lambda = [&caseID, &controlPoint, this](const SemanticTypes::Lesion& lesion) { return !ControlPointContainsLesion(caseID, lesion, controlPoint); };
  allLesions.erase(std::remove_if(allLesions.begin(), allLesions.end(), lambda), allLesions.end());

  return allLesions;
}

mitk::SemanticRelations::LesionVector mitk::SemanticRelations::GetAllLesionsInImage(const DataNode* imageNode) const
{
  if (nullptr == imageNode)
  {
    mitkThrow() << "Not a valid image data node.";
  }

  LesionVector allLesionsInImage;
  if (m_DataStorage.IsNotNull())
  {
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
  }
  return allLesionsInImage;
}

mitk::SemanticTypes::Lesion mitk::SemanticRelations::GetRepresentedLesion(const DataNode* segmentationNode) const
{
  if (nullptr == segmentationNode)
  {
    mitkThrow() << "Not a valid segmentation data node.";
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
  catch (const SemanticRelationException&)
  {
    return false;
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e);
  }
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllSegmentationsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  if (InstanceExists(caseID, lesion))
  {
    // lesion exists, retrieve all case segmentations from the storage
    DataNodeVector allSegmentationsOfLesion = m_RelationStorage->GetAllSegmentationsOfCase(caseID);

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

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllImagesOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  DataNodeVector allDataOfLesion;
  if (m_DataStorage.IsNotNull())
  {
    // 1. get all segmentations that define the lesion
    // 2. retrieve the parent node (source) of the found segmentation node
    try
    {
      DataNodeVector allSegmentationsOfLesion = GetAllSegmentationsOfLesion(caseID, lesion);
      for (const auto& segmentationNode : allSegmentationsOfLesion)
      {
        // get parent node of the current segmentation node with the node predicate
        DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage->GetSources(segmentationNode, NodePredicates::GetImagePredicate(), false);
        for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
        {
          DataNode::Pointer dataNode = it->Value();
          allDataOfLesion.push_back(it->Value());
        }
      }
      return allDataOfLesion;
    }
    catch (SemanticRelationException& e)
    {
      mitkReThrow(e);
    }
  }
  else
  {
    mitkThrow() << "Not a valid data storage.";
  }
}

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  LesionVector allLesions = GetAllLesionsOfCase(caseID);

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

mitk::SemanticRelations::ControlpointVector mitk::SemanticRelations::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllControlPointsOfCase(caseID);
}

mitk::SemanticRelations::ControlpointVector mitk::SemanticRelations::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion) const
{
  ControlpointVector allControlPoints = GetAllControlPointsOfCase(caseID);

  // filter the control points: use only those, where the associated image data has a segmentation that refers to the given lesion using a lambda function
  auto lambda = [&caseID, &lesion, this](const SemanticTypes::ControlPoint& controlPoint) { return !ControlPointContainsLesion(caseID, lesion, controlPoint); };
  allControlPoints.erase(std::remove_if(allControlPoints.begin(), allControlPoints.end(), lambda), allControlPoints.end());

  return allControlPoints;
}

mitk::SemanticRelations::ControlpointVector mitk::SemanticRelations::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const
{
  ControlpointVector allControlPoints = GetAllControlPointsOfCase(caseID);

  // filter the control points: use only those, where the associated image data refers to the given information type using a lambda function
  auto lambda = [&caseID, &informationType, this](const SemanticTypes::ControlPoint& controlPoint) { return !ControlPointContainsInformationType(caseID, informationType, controlPoint); };
  allControlPoints.erase(std::remove_if(allControlPoints.begin(), allControlPoints.end(), lambda), allControlPoints.end());

  return allControlPoints;
}

mitk::SemanticTypes::ControlPoint mitk::SemanticRelations::GetControlPointOfData(const DataNode* dataNode) const
{
  if (nullptr == dataNode)
  {
    mitkThrow() << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  SemanticTypes::ID dataNodeID = GetIDFromDataNode(dataNode);
  return m_RelationStorage->GetControlPointOfData(caseID, dataNodeID);
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllDataOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  if (InstanceExists(caseID, controlPoint))
  {
    // control point exists, retrieve all images from the storage
    DataNodeVector allDataOfControlPoint = m_RelationStorage->GetAllImagesOfCase(caseID);

    // filter all images to remove the ones with a different control point using a lambda function
    auto lambda = [&controlPoint, this](DataNode::Pointer imageNode) { return controlPoint.UID != GetControlPointOfData(imageNode).UID; };
    allDataOfControlPoint.erase(std::remove_if(allDataOfControlPoint.begin(), allDataOfControlPoint.end(), lambda), allDataOfControlPoint.end());

    return allDataOfControlPoint;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing control point instance for the given caseID " << caseID << " and control point " << controlPoint.UID << ".";
  }
}

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  ControlpointVector allControlPoints = GetAllControlPointsOfCase(caseID);

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

mitk::SemanticRelations::InformationTypeVector mitk::SemanticRelations::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID) const
{
  return m_RelationStorage->GetAllInformationTypesOfCase(caseID);
}

mitk::SemanticRelations::InformationTypeVector mitk::SemanticRelations::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint) const
{
  InformationTypeVector allInformationTypes = GetAllInformationTypesOfCase(caseID);

  // filter the information types: use only those, where the associated data refers to the given control point using a lambda function
  auto lambda = [&caseID, &controlPoint, this](const SemanticTypes::InformationType& informationType) { return !ControlPointContainsInformationType(caseID, informationType, controlPoint); };
  allInformationTypes.erase(std::remove_if(allInformationTypes.begin(), allInformationTypes.end(), lambda), allInformationTypes.end());

  return allInformationTypes;
}

mitk::SemanticTypes::InformationType mitk::SemanticRelations::GetInformationTypeOfImage(const DataNode* imageNode) const
{
  if (nullptr == imageNode)
  {
    mitkThrow() << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID imageID = GetIDFromDataNode(imageNode);
  return m_RelationStorage->GetInformationTypeOfImage(caseID, imageID);
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetAllDataOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const
{
  if (InstanceExists(caseID, informationType))
  {
    // information type exists, retrieve all images from the storage
    DataNodeVector allDataOfInformationType = m_RelationStorage->GetAllImagesOfCase(caseID);

    // filter all images to remove the ones with a different information type using a lambda function
    auto lambda = [&informationType, this](DataNode::Pointer imageNode) { return informationType != GetInformationTypeOfImage(imageNode); };
    allDataOfInformationType.erase(std::remove_if(allDataOfInformationType.begin(), allDataOfInformationType.end(), lambda), allDataOfInformationType.end());

    return allDataOfInformationType;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing information type for the given caseID " << caseID << " and information type " << informationType;
  }
}

mitk::SemanticRelations::DataNodeVector mitk::SemanticRelations::GetFilteredData(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::InformationType& informationType) const
{
  if (InstanceExists(caseID, controlPoint))
  {
    if (InstanceExists(caseID, informationType))
    {
      // control point exists, information type exists, retrieve all images from the storage
      DataNodeVector allImagesOfCase = m_RelationStorage->GetAllImagesOfCase(caseID);
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

bool mitk::SemanticRelations::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType) const
{
  InformationTypeVector allInformationTypes = GetAllInformationTypesOfCase(caseID);

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

void mitk::SemanticRelations::AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (InstanceExists(caseID, lesion))
  {
    mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to add already exists for the given case.";
  }
  else
  {
    m_RelationStorage->AddLesion(caseID, lesion);
  }
}

void mitk::SemanticRelations::OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (InstanceExists(caseID, lesion))
  {
    m_RelationStorage->OverwriteLesion(caseID, lesion);
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "The lesion " << lesion.UID << " to overwrite does not exist for the given case.";
  }
}

void mitk::SemanticRelations::AddLesionAndLinkData(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion)
{
  if (nullptr == segmentationNode)
  {
    mitkThrow() << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  try
  {
    AddLesion(caseID, lesion);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e);
  }

  try
  {
    LinkSegmentationToLesion(segmentationNode, lesion);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e);
  }
}

void mitk::SemanticRelations::LinkSegmentationToLesion(const DataNode* segmentationNode, const SemanticTypes::Lesion& lesion)
{
  if (nullptr == segmentationNode)
  {
    mitkThrow() << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  if (InstanceExists(caseID, lesion))
  {
    SemanticTypes::ID segmentationID = GetIDFromDataNode(segmentationNode);
    m_RelationStorage->LinkSegmentationToLesion(caseID, segmentationID, lesion);
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
    mitkThrow() << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationID = GetIDFromDataNode(segmentationNode);
  mitk::SemanticTypes::Lesion lesion = m_RelationStorage->GetRepresentedLesion(caseID, segmentationID);
  m_RelationStorage->UnlinkSegmentationFromLesion(caseID, segmentationID);
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

void mitk::SemanticRelations::AddControlPointAndLinkData(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence)
{
  if (nullptr == dataNode)
  {
    mitkThrow() << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  if (InstanceExists(caseID, controlPoint))
  {
    mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to add already exists for the given case. \n Use 'LinkDataToControlPoint' instead.";
  }

  // control point does not already exist
  bool contained = CheckContainingControlPoint(caseID, controlPoint);
  if (contained)
  {
    mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to add is already contained in an existing control point.";
  }

  // control point is not already contained in an existing control point
  if (checkConsistence)
  {
    // to check the consistency, the control point manager checks, if the date extracted from the data node is inside the given control point
    bool insideControlPoint = InsideControlPoint(dataNode, controlPoint);
    if (!insideControlPoint)
    {
      mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to add does not contain the date of the given data node.";
    }
  }
  m_RelationStorage->AddControlPoint(caseID, controlPoint);

  try
  {
    LinkDataToControlPoint(dataNode, controlPoint, checkConsistence);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e);
  }
}

void mitk::SemanticRelations::OverwriteControlPointAndLinkData(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence)
{
  if (nullptr == dataNode)
  {
    mitkThrow() << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);

  ControlpointVector allControlPoints = GetAllControlPointsOfCase(caseID);
  const auto existingControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(),
    [&controlPoint](const SemanticTypes::ControlPoint& currentControlPoint) { return currentControlPoint.UID == controlPoint.UID; });

  if (existingControlPoint != allControlPoints.end())
  {
    // control point does already exist
    if (checkConsistence)
    {
      // to check the consistency, the control point manager checks, if the date extracted from the data node is inside the given control point
      bool insideControlPoint = InsideControlPoint(dataNode, controlPoint);
      if (!insideControlPoint)
      {
        mitkThrowException(SemanticRelationException) << "The overwriting control point " << controlPoint.UID << " does not contain the date of the given data node.";
      }
    }

    bool sameStartPoint = controlPoint.startPoint == (*existingControlPoint).startPoint;
    bool sameEndPoint = controlPoint.endPoint == (*existingControlPoint).endPoint;
    if (!sameStartPoint && !sameEndPoint)
    {
      mitkThrowException(SemanticRelationException) << "The overwriting control point " << controlPoint.UID << " differs in the start date and in the end date from the original control point.";
    }
    if (sameStartPoint && sameEndPoint)
    {
      mitkThrowException(SemanticRelationException) << "The overwriting control point " << controlPoint.UID << " does not differ from the original control point.";
    }

    bool overlapping = CheckOverlappingControlPoint(caseID, controlPoint);
    if (overlapping)
    {
      mitkThrowException(SemanticRelationException) << "The overwriting control point " << controlPoint.UID << " overlaps with an already existing control point";
    }
    else
    {
      m_RelationStorage->OverwriteControlPoint(caseID, controlPoint);

      try
      {
        LinkDataToControlPoint(dataNode, controlPoint, checkConsistence);
      }
      catch (SemanticRelationException& e)
      {
        mitkReThrow(e);
      }
    }
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "The control point " << controlPoint.UID << " to overwrite does not exist for the given case. \n Use 'AddControlPointAndLinkData' instead.";
  }
}

void mitk::SemanticRelations::LinkDataToControlPoint(const DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint, bool checkConsistence)
{
  if (nullptr == dataNode)
  {
    mitkThrow() << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  if (InstanceExists(caseID, controlPoint))
  {
    SemanticTypes::ID dataID = GetIDFromDataNode(dataNode);

    // control point does already exist
    if (checkConsistence)
    {
      // to check the consistency, the control point manager checks, if the date extracted from the data node is inside the given control point
      bool insideControlPoint = InsideControlPoint(dataNode, controlPoint);
      if (!insideControlPoint)
      {
        mitkThrowException(SemanticRelationException) << "The data to link does not lie inside the given control point " << controlPoint.UID << " .";
      }
    }
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
    mitkThrow() << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
  SemanticTypes::ID dataID = GetIDFromDataNode(dataNode);
  mitk::SemanticTypes::ControlPoint controlPoint = m_RelationStorage->GetControlPointOfData(caseID, dataID);
  m_RelationStorage->UnlinkDataFromControlPoint(caseID, dataID);

  try
  {
    DataNodeVector allDataOfControlPoint = GetAllDataOfControlPoint(caseID, controlPoint);
    if (allDataOfControlPoint.empty())
    {
      // no more data is linked to the specific control point
      // the control point can be removed from the storage
      m_RelationStorage->RemoveControlPointFromCase(caseID, controlPoint);
    }
    else
    {
      // some data is still linked to this control point
      // the control point can not be removed, but has to be adjusted to fit the remaining data
      SemanticTypes::ControlPoint adjustedControlPoint = GenerateControlPoint(allDataOfControlPoint);
      // set the UIDs to be the same, so that all references still work
      adjustedControlPoint.UID = controlPoint.UID;
      adjustedControlPoint.startPoint.UID = controlPoint.startPoint.UID;
      adjustedControlPoint.endPoint.UID = controlPoint.endPoint.UID;
      m_RelationStorage->OverwriteControlPoint(caseID, adjustedControlPoint);
    }
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e);
  }
}

void mitk::SemanticRelations::AddInformationTypeToImage(const DataNode* imageNode, const SemanticTypes::InformationType& informationType)
{
  if (nullptr == imageNode)
  {
    mitkThrow() << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID imageID = GetIDFromDataNode(imageNode);
  m_RelationStorage->AddInformationTypeToImage(caseID, imageID, informationType);
}

void mitk::SemanticRelations::RemoveInformationTypeFromImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrow() << "Not a valid image data node.";
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
bool mitk::SemanticRelations::ControlPointContainsLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint) const
{
  DataNodeVector allDataOfLesion;
  try
  {
    allDataOfLesion = GetAllImagesOfLesion(caseID, lesion);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving image data; lesion has to be outside the control point
    return false;
  }

  DataNodeVector allDataOfControlPoint;
  try
  {
    allDataOfControlPoint = GetAllDataOfControlPoint(caseID, controlPoint);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving control point data; lesion has to be outside the control point
    return false;
  }

  std::sort(allDataOfLesion.begin(), allDataOfLesion.end());
  std::sort(allDataOfControlPoint.begin(), allDataOfControlPoint.end());
  DataNodeVector allDataIntersection;
  // set intersection removes duplicated nodes, since 'allDataOfControlPoint' only contains at most one of each node
  std::set_intersection(allDataOfLesion.begin(), allDataOfLesion.end(),
                        allDataOfControlPoint.begin(), allDataOfControlPoint.end(),
                        std::back_inserter(allDataIntersection));

  // if the vector of intersecting data is empty, the control point does not contain the lesion
  return !allDataIntersection.empty();
}

bool mitk::SemanticRelations::ControlPointContainsInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ControlPoint& controlPoint) const
{
  DataNodeVector allDataOfInformationType;
  try
  {
    allDataOfInformationType = GetAllDataOfInformationType(caseID, informationType);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving image data; information type has to be outside the control point
    return false;
  }

  DataNodeVector allDataOfControlPoint;
  try
  {
    allDataOfControlPoint = GetAllDataOfControlPoint(caseID, controlPoint);
  }
  catch (const SemanticRelationException&)
  {
    // error retrieving control point data; information type has to be outside the control point
    return false;
  }

  std::sort(allDataOfInformationType.begin(), allDataOfInformationType.end());
  std::sort(allDataOfControlPoint.begin(), allDataOfControlPoint.end());
  DataNodeVector allDataIntersection;
  std::set_intersection(allDataOfInformationType.begin(), allDataOfInformationType.end(),
                        allDataOfControlPoint.begin(), allDataOfControlPoint.end(),
                        std::back_inserter(allDataIntersection));
  return !allDataIntersection.empty();
}

bool mitk::SemanticRelations::CheckOverlappingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  ControlpointVector allControlPoints = GetAllControlPointsOfCase(caseID);
  if (allControlPoints.empty())
  {
    return false;
  }

  std::sort(allControlPoints.begin(), allControlPoints.end());
  const auto existingControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(),
    [&controlPoint](const SemanticTypes::ControlPoint& currentControlPoint) { return currentControlPoint.UID == controlPoint.UID; });

  auto nextControlPoint = allControlPoints.end();
  auto previousControlPoint = allControlPoints.begin();
  if (existingControlPoint != allControlPoints.end())
  {
    // case overwriting: control point already contained in the list of all existing control points
    nextControlPoint = std::next(existingControlPoint, 1);
    if (existingControlPoint != allControlPoints.begin())
    {
      previousControlPoint = std::prev(existingControlPoint, 1);
    }
  }
  else
  {
    // case adding: control point not contained in the list of all existing control points
    nextControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(),
      [&controlPoint](const SemanticTypes::ControlPoint& currentControlPoint) { return currentControlPoint.startPoint >= controlPoint.endPoint; });
    if (nextControlPoint != allControlPoints.begin())
    {
      previousControlPoint = std::prev(existingControlPoint, 1);
    }
  }

  // check the neighboring control points for overlap
  bool overlapWithNext = false;
  bool overlapWithPrevious = CheckForOverlap(controlPoint, *previousControlPoint);

  if (nextControlPoint != allControlPoints.end())
  {
    overlapWithNext = CheckForOverlap(controlPoint, *nextControlPoint);
  }

  return overlapWithNext || overlapWithPrevious; // return true if at least one overlap is detected
}

bool mitk::SemanticRelations::CheckContainingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  ControlpointVector allControlPoints = GetAllControlPointsOfCase(caseID);
  if (allControlPoints.empty())
  {
    return false;
  }

  const auto existingControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(),
    [&controlPoint](const SemanticTypes::ControlPoint& currentControlPoint) { return currentControlPoint.UID == controlPoint.UID; });

  if (existingControlPoint != allControlPoints.end())
  {
    // case overwriting: control point already contained in the list of all existing control points
    // -> duplicated control point found (regardless of the actual start point and end point)
    return true;
  }
  else
  {
    // case adding: control point not contained in the list of all existing control points
    for (const auto& existingControlPoint : allControlPoints)
    {
      bool contained = InsideControlPoint(controlPoint, existingControlPoint);
      if (contained)
      {
        return true;
      }
    }
  }
  return false;
}
