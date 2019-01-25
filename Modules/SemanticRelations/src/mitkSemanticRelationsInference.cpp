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

#include "mitkSemanticRelationsInference.h"

// semantic relations module
#include "mitkControlPointManager.h"
#include "mitkDICOMHelper.h"
#include "mitkNodePredicates.h"
#include "mitkRelationStorage.h"
#include "mitkSemanticRelationException.h"

/************************************************************************/
/* functions to get instances / attributes                              */
/************************************************************************/

mitk::SemanticTypes::LesionVector mitk::SemanticRelationsInference::GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID)
{
  return RelationStorage::GetAllLesionsOfCase(caseID);
}

mitk::SemanticTypes::LesionClassVector mitk::SemanticRelationsInference::GetAllLesionClassesOfCase(const SemanticTypes::CaseID& caseID)
{
  SemanticTypes::LesionVector allLesionsOfCase = GetAllLesionsOfCase(caseID);
  SemanticTypes::LesionClassVector allLesionClassesOfCase;

  for (const auto& lesion : allLesionsOfCase)
  {
    allLesionClassesOfCase.push_back(lesion.lesionClass);
  }

  // remove duplicate entries
  auto lessThan = [](const SemanticTypes::LesionClass& lesionClassLeft, const SemanticTypes::LesionClass& lesionClassRight)
  {
    return lesionClassLeft.UID < lesionClassRight.UID;
  };

  auto equal = [](const SemanticTypes::LesionClass& lesionClassLeft, const SemanticTypes::LesionClass& lesionClassRight)
  {
    return lesionClassLeft.UID == lesionClassRight.UID;
  };

  std::sort(allLesionClassesOfCase.begin(), allLesionClassesOfCase.end(), lessThan);
  allLesionClassesOfCase.erase(std::unique(allLesionClassesOfCase.begin(), allLesionClassesOfCase.end(), equal), allLesionClassesOfCase.end());

  return allLesionClassesOfCase;
}

mitk::SemanticTypes::Lesion mitk::SemanticRelationsInference::GetRepresentedLesion(const DataNode* segmentationNode)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(segmentationNode);
  SemanticTypes::ID segmentationID = GetIDFromDataNode(segmentationNode);
  SemanticTypes::Lesion representedLesion = GetRepresentedLesion(caseID, segmentationID);

  if (representedLesion.UID.empty())
  {
    mitkThrowException(SemanticRelationException) << "Could not find a represented lesion instance for the given segmentation node " << segmentationNode->GetName();
  }
  else
  {
    return representedLesion;
  }
}

mitk::SemanticTypes::Lesion mitk::SemanticRelationsInference::GetRepresentedLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  SemanticTypes::Lesion representedLesion = RelationStorage::GetRepresentedLesion(caseID, segmentationID);

  if (representedLesion.UID.empty())
  {
    mitkThrowException(SemanticRelationException) << "Could not find a represented lesion instance for the given segmentation node ID " << segmentationID;
  }
  else
  {
    return representedLesion;
  }
}

bool mitk::SemanticRelationsInference::IsRepresentingALesion(const DataNode* segmentationNode)
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

bool mitk::SemanticRelationsInference::IsRepresentingALesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  try
  {
    SemanticTypes::Lesion representedLesion = GetRepresentedLesion(caseID, segmentationID);
    return true;
  }
  catch (const Exception&)
  {
    return false;
  }
}

bool mitk::SemanticRelationsInference::IsLesionPresentOnSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ID& segmentationID)
{
  try
  {
    const auto representedLesion = GetRepresentedLesion(caseID, segmentationID);
    return lesion.UID == representedLesion.UID;
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }
}

bool mitk::SemanticRelationsInference::InstanceExists(const DataNode* dataNode)
{
  try
  {
    SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(dataNode);
    SemanticTypes::ID dataNodeID = GetIDFromDataNode(dataNode);

    if (NodePredicates::GetImagePredicate()->CheckNode(dataNode))
    {
      std::vector<std::string> allImageIDsOfCase = RelationStorage::GetAllImageIDsOfCase(caseID);
      return std::find(allImageIDsOfCase.begin(), allImageIDsOfCase.end(), dataNodeID) != allImageIDsOfCase.end();
    }
    else if (NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
    {
      std::vector<std::string> allSegmentationIDsOfCase = RelationStorage::GetAllSegmentationIDsOfCase(caseID);
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

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  SemanticTypes::LesionVector allLesions = GetAllLesionsOfCase(caseID);

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

mitk::SemanticTypes::IDVector mitk::SemanticRelationsInference::GetAllSegmentationIDsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (InstanceExists(caseID, lesion))
  {
    // lesion exists, retrieve all case segmentations from the storage
    SemanticTypes::IDVector allSegmentationIDs = RelationStorage::GetAllSegmentationIDsOfCase(caseID);

    // filter all segmentationIDs: check for semantic relation with the given lesion using a lambda function
    auto lambda = [&lesion, caseID](const SemanticTypes::ID& segmentationID)
    {
      try
      {
        SemanticTypes::Lesion representedLesion = GetRepresentedLesion(caseID, segmentationID);
        return lesion.UID != representedLesion.UID;
      }
      catch (const SemanticRelationException&)
      {
        return true;
      }
    };

    allSegmentationIDs.erase(std::remove_if(allSegmentationIDs.begin(), allSegmentationIDs.end(), lambda), allSegmentationIDs.end());

    return allSegmentationIDs;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing lesion instance for the given caseID " << caseID << " and lesion " << lesion.UID << ".";
  }
}

mitk::SemanticTypes::ControlPointVector mitk::SemanticRelationsInference::GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID)
{
  return RelationStorage::GetAllControlPointsOfCase(caseID);
}

mitk::SemanticTypes::ControlPoint mitk::SemanticRelationsInference::GetControlPointOfImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID dataNodeID = GetIDFromDataNode(imageNode);
  return GetControlPointOfImage(caseID, dataNodeID);
}

mitk::SemanticTypes::ControlPoint mitk::SemanticRelationsInference::GetControlPointOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  return RelationStorage::GetControlPointOfImage(caseID, imageID);
}

mitk::SemanticTypes::IDVector mitk::SemanticRelationsInference::GetAllImageIDsOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  if (InstanceExists(caseID, controlPoint))
  {
    // control point exists, retrieve all imageIDs from the storage
    SemanticTypes::IDVector allImageIDs = RelationStorage::GetAllImageIDsOfCase(caseID);

    // filter all imageIDs to remove the ones with a different control point using a lambda function
    auto lambda = [&controlPoint, &caseID](const SemanticTypes::ID& imageID)
    {
      return controlPoint.UID != RelationStorage::GetControlPointOfImage(caseID, imageID).UID;
    };

    allImageIDs.erase(std::remove_if(allImageIDs.begin(), allImageIDs.end(), lambda), allImageIDs.end());

    return allImageIDs;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing control point instance for the given caseID " << caseID << " and control point " << controlPoint.UID << ".";
  }
}

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
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

mitk::SemanticTypes::ExaminationPeriodVector mitk::SemanticRelationsInference::GetAllExaminationPeriodsOfCase(const SemanticTypes::CaseID& caseID)
{
  return RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
}

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod)
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

mitk::SemanticTypes::InformationTypeVector mitk::SemanticRelationsInference::GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID)
{
  return RelationStorage::GetAllInformationTypesOfCase(caseID);
}

mitk::SemanticTypes::InformationType mitk::SemanticRelationsInference::GetInformationTypeOfImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = GetCaseIDFromDataNode(imageNode);
  SemanticTypes::ID imageID = GetIDFromDataNode(imageNode);
  return GetInformationTypeOfImage(caseID, imageID);
}

mitk::SemanticTypes::InformationType mitk::SemanticRelationsInference::GetInformationTypeOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID)
{
  return RelationStorage::GetInformationTypeOfImage(caseID, imageID);
}

mitk::SemanticTypes::IDVector mitk::SemanticRelationsInference::GetAllImageIDsOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType)
{
  if (InstanceExists(caseID, informationType))
  {
    // information type exists, retrieve all imageIDs from the storage
    SemanticTypes::IDVector allImageIDs = RelationStorage::GetAllImageIDsOfCase(caseID);

    // filter all images to remove the ones with a different information type using a lambda function
    auto lambda = [&informationType, caseID](const SemanticTypes::ID& imageID)
    {
      return informationType != RelationStorage::GetInformationTypeOfImage(caseID, imageID);
    };

    allImageIDs.erase(std::remove_if(allImageIDs.begin(), allImageIDs.end(), lambda), allImageIDs.end());

    return allImageIDs;
  }
  else
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing information type for the given caseID " << caseID << " and information type " << informationType;
  }
}

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType)
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

std::vector<mitk::SemanticTypes::CaseID> mitk::SemanticRelationsInference::GetAllCaseIDs()
{
  return RelationStorage::GetAllCaseIDs();
}

void mitk::SemanticRelationsInference::ClearControlPoints(const SemanticTypes::CaseID& caseID)
{
  SemanticTypes::ControlPointVector allControlPointsOfCase = GetAllControlPointsOfCase(caseID);

  std::vector<std::string> allImageIDsVectorValue = RelationStorage::GetAllImageIDsOfCase(caseID);
  SemanticTypes::ControlPointVector referencedControlPoints;
  for (const auto& imageID : allImageIDsVectorValue)
  {
    auto controlPointOfImage = GetControlPointOfImage(caseID, imageID);
    referencedControlPoints.push_back(controlPointOfImage);
  }

  std::sort(allControlPointsOfCase.begin(), allControlPointsOfCase.end());
  std::sort(referencedControlPoints.begin(), referencedControlPoints.end());

  SemanticTypes::ControlPointVector nonReferencedControlPoints;
  std::set_difference(allControlPointsOfCase.begin(), allControlPointsOfCase.end(),
                      referencedControlPoints.begin(), referencedControlPoints.end(),
                      std::inserter(nonReferencedControlPoints, nonReferencedControlPoints.begin()));

  auto allExaminationPeriods = GetAllExaminationPeriodsOfCase(caseID);
  for (const auto& controlPoint : nonReferencedControlPoints)
  {
    const auto& examinationPeriod = FindExaminationPeriod(controlPoint, allExaminationPeriods);
    RelationStorage::RemoveControlPointFromExaminationPeriod(caseID, controlPoint, examinationPeriod);
    RelationStorage::RemoveControlPointFromCase(caseID, controlPoint);
  }
}
