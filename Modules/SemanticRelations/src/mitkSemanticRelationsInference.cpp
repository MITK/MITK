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

mitk::SemanticTypes::LesionClassVector mitk::SemanticRelationsInference::GetAllLesionClassesOfCase(const SemanticTypes::CaseID& caseID)
{
  SemanticTypes::LesionVector allLesionsOfCase = RelationStorage::GetAllLesionsOfCase(caseID);
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

mitk::SemanticTypes::Lesion mitk::SemanticRelationsInference::GetLesionOfSegmentation(const DataNode* segmentationNode)
{
  if (nullptr == segmentationNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid segmentation data node.";
  }

  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ID segmentationID = "";
  try
  {
    caseID = GetCaseIDFromDataNode(segmentationNode);
    segmentationID = GetIDFromDataNode(segmentationNode);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get the lesion of the given segmentation data node.";
  }

  return RelationStorage::GetLesionOfSegmentation(caseID, segmentationID);
}

mitk::SemanticTypes::LesionVector mitk::SemanticRelationsInference::GetAllLesionsOfImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ID imageID = "";
  try
  {
    caseID = GetCaseIDFromDataNode(imageNode);
    imageID = GetIDFromDataNode(imageNode);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get all lesions of the given image data node.";
  }

  SemanticTypes::LesionVector allLesionsOfImage;
  // 1. get all segmentations that are connected to the given image
  // 2. get the lesion of each segmentation
  // 3. guarantee uniqueness of lesions
  SemanticTypes::IDVector allSegmentationIDsOfImage = RelationStorage::GetAllSegmentationIDsOfImage(caseID, imageID);
  for (const auto& segmentationID : allSegmentationIDsOfImage)
  {
    // get represented lesion of the current segmentation
    SemanticTypes::Lesion representedLesion = RelationStorage::GetLesionOfSegmentation(caseID, segmentationID);
    if (!representedLesion.UID.empty())
    {
      allLesionsOfImage.push_back(representedLesion);
    }
  }

  // remove duplicate entries
  auto lessThan = [](const SemanticTypes::Lesion& lesionLeft, const SemanticTypes::Lesion& lesionRight)
  {
    return lesionLeft.UID < lesionRight.UID;
  };

  auto equal = [](const SemanticTypes::Lesion& lesionLeft, const SemanticTypes::Lesion& lesionRight)
  {
    return lesionLeft.UID == lesionRight.UID;
  };

  std::sort(allLesionsOfImage.begin(), allLesionsOfImage.end(), lessThan);
  allLesionsOfImage.erase(std::unique(allLesionsOfImage.begin(), allLesionsOfImage.end(), equal), allLesionsOfImage.end());

  return allLesionsOfImage;
}

mitk::SemanticTypes::LesionVector mitk::SemanticRelationsInference::GetAllLesionsOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::LesionVector allLesions = RelationStorage::GetAllLesionsOfCase(caseID);

  // filter the lesions: use only those, where the associated data is connected to image data that refers to the given control point using a lambda function
  auto lambda = [&caseID, &controlPoint](const SemanticTypes::Lesion& lesion)
  {
    return !SpecificImageExists(caseID, lesion, controlPoint);
  };

  allLesions.erase(std::remove_if(allLesions.begin(), allLesions.end(), lambda), allLesions.end());

  return allLesions;
}

bool mitk::SemanticRelationsInference::IsRepresentingALesion(const DataNode* segmentationNode)
{
  SemanticTypes::Lesion representedLesion;
  try
  {
    representedLesion = GetLesionOfSegmentation(segmentationNode);
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }

  return !representedLesion.UID.empty();
}

bool mitk::SemanticRelationsInference::IsRepresentingALesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID)
{
  SemanticTypes::Lesion representedLesion = RelationStorage::GetLesionOfSegmentation(caseID, segmentationID);
  return !representedLesion.UID.empty();
}

bool mitk::SemanticRelationsInference::IsLesionPresent(const SemanticTypes::Lesion& lesion, const DataNode* dataNode)
{
  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ID dataNodeID = "";
  try
  {
    caseID = GetCaseIDFromDataNode(dataNode);
    dataNodeID = GetIDFromDataNode(dataNode);
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }

  if (NodePredicates::GetImagePredicate()->CheckNode(dataNode))
  {
    return IsLesionPresentOnImage(caseID, lesion, dataNodeID);
  }

  if (NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
  {
    return IsLesionPresentOnSegmentation(caseID, lesion, dataNodeID);
  }

  return false;
}

bool mitk::SemanticRelationsInference::IsLesionPresentOnImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ID& imageID)
{
  SemanticTypes::IDVector allImageIDsOfLesion;
  try
  {
    allImageIDsOfLesion = GetAllImageIDsOfLesion(caseID, lesion);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get all image IDs of the given lesion to determine the lesion presence.";
  }

  for (const auto& imageIDOfLesion : allImageIDsOfLesion)
  {
    if (imageIDOfLesion == imageID)
    {
      return true;
    }
  }

  return false;
}

bool mitk::SemanticRelationsInference::IsLesionPresentOnSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ID& segmentationID)
{
  const auto representedLesion = RelationStorage::GetLesionOfSegmentation(caseID, segmentationID);
  return lesion.UID == representedLesion.UID;
}

bool mitk::SemanticRelationsInference::IsLesionPresentAtControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::IDVector allImageIDsOfLesion;
  try
  {
    allImageIDsOfLesion = GetAllImageIDsOfLesion(caseID, lesion);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get all image IDs of the given lesion to determine the lesion presence.";
  }

  for (const auto& imageIDOfLesion : allImageIDsOfLesion)
  {
    auto imageControlPoint = mitk::RelationStorage::GetControlPointOfImage(caseID, imageIDOfLesion);
    if (imageControlPoint.date == controlPoint.date)
    {
      return true;
    }
  }

  return false;
}

bool mitk::SemanticRelationsInference::InstanceExists(const DataNode* dataNode)
{
  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ID dataNodeID = "";
  try
  {
    caseID = GetCaseIDFromDataNode(dataNode);
    dataNodeID = GetIDFromDataNode(dataNode);
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }

  if (NodePredicates::GetImagePredicate()->CheckNode(dataNode))
  {
    std::vector<std::string> allImageIDsOfCase = RelationStorage::GetAllImageIDsOfCase(caseID);
    return std::find(allImageIDsOfCase.begin(), allImageIDsOfCase.end(), dataNodeID) != allImageIDsOfCase.end();
  }

  if (NodePredicates::GetSegmentationPredicate()->CheckNode(dataNode))
  {
    std::vector<std::string> allSegmentationIDsOfCase = RelationStorage::GetAllSegmentationIDsOfCase(caseID);
    return std::find(allSegmentationIDsOfCase.begin(), allSegmentationIDsOfCase.end(), dataNodeID) != allSegmentationIDsOfCase.end();
  }

  return false;
}

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  SemanticTypes::LesionVector allLesions = RelationStorage::GetAllLesionsOfCase(caseID);

  // filter all lesions: check for equality with the given lesion using a lambda function
  auto lambda = [&lesion](const SemanticTypes::Lesion& currentLesion)
  {
    return currentLesion.UID == lesion.UID;
  };

  const auto existingLesion = std::find_if(allLesions.begin(), allLesions.end(), lambda);

  return existingLesion != allLesions.end();
}

mitk::SemanticTypes::IDVector mitk::SemanticRelationsInference::GetAllImageIDsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  if (!InstanceExists(caseID, lesion))
  {
    mitkThrowException(SemanticRelationException) << "Could not find an existing lesion instance for the given caseID " << caseID << " and lesion " << lesion.UID << ".";
  }

  SemanticTypes::IDVector allImageIDsOfLesion;
  // 1. get all segmentations that define the lesion
  // 2. get the parentID (imageID) of each segmentation
  // 3. guarantee uniqueness of image IDs
  SemanticTypes::IDVector allSegmentationIDsOfLesion = RelationStorage::GetAllSegmentationIDsOfLesion(caseID, lesion);
  for (const auto& segmentationID : allSegmentationIDsOfLesion)
  {
    // get parent ID of the current segmentation ID
    SemanticTypes::ID imageID = RelationStorage::GetImageIDOfSegmentation(caseID, segmentationID);
    if(!imageID.empty())
    {
      allImageIDsOfLesion.push_back(imageID);
    }
  }

  std::sort(allImageIDsOfLesion.begin(), allImageIDsOfLesion.end());
  allImageIDsOfLesion.erase(std::unique(allImageIDsOfLesion.begin(), allImageIDsOfLesion.end()), allImageIDsOfLesion.end());

  return allImageIDsOfLesion;
}

mitk::SemanticTypes::ControlPoint mitk::SemanticRelationsInference::GetControlPointOfImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ID imageID = "";
  try
  {
    caseID = GetCaseIDFromDataNode(imageNode);
    imageID = GetIDFromDataNode(imageNode);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get the control point of the given image data node.";
  }

  return RelationStorage::GetControlPointOfImage(caseID, imageID);
}

mitk::SemanticTypes::ControlPointVector mitk::SemanticRelationsInference::GetAllControlPointsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion)
{
  SemanticTypes::ControlPointVector allControlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);

  // filter the control points: use only those, where the associated image data has a segmentation that refers to the given lesion using a lambda function
  auto lambda = [&caseID, &lesion](const SemanticTypes::ControlPoint& controlPoint)
  {
    return !SpecificImageExists(caseID, lesion, controlPoint);
  };

  allControlPoints.erase(std::remove_if(allControlPoints.begin(), allControlPoints.end(), lambda), allControlPoints.end());

  return allControlPoints;
}

mitk::SemanticTypes::ControlPointVector mitk::SemanticRelationsInference::GetAllControlPointsOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType)
{
  SemanticTypes::ControlPointVector allControlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);

  // filter the control points: use only those, where the associated image data refers to the given information type using a lambda function
  auto lambda = [&caseID, &informationType](const SemanticTypes::ControlPoint& controlPoint)
  {
    return !SpecificImageExists(caseID, informationType, controlPoint);
  };

  allControlPoints.erase(std::remove_if(allControlPoints.begin(), allControlPoints.end(), lambda), allControlPoints.end());

  return allControlPoints;
}

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::ControlPointVector allControlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);

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

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod)
{
  SemanticTypes::ExaminationPeriodVector allExaminationPeriods = RelationStorage::GetAllExaminationPeriodsOfCase(caseID);

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

mitk::SemanticTypes::InformationType mitk::SemanticRelationsInference::GetInformationTypeOfImage(const DataNode* imageNode)
{
  if (nullptr == imageNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid image data node.";
  }

  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ID imageID = "";
  try
  {
    caseID = GetCaseIDFromDataNode(imageNode);
    imageID = GetIDFromDataNode(imageNode);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot get the information type of the given image data node.";
  }

  return RelationStorage::GetInformationTypeOfImage(caseID, imageID);
}

mitk::SemanticTypes::InformationTypeVector mitk::SemanticRelationsInference::GetAllInformationTypesOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::InformationTypeVector allInformationTypes = RelationStorage::GetAllInformationTypesOfCase(caseID);

  // filter the information types: use only those, where the associated data refers to the given control point using a lambda function
  auto lambda = [&caseID, &controlPoint](const SemanticTypes::InformationType& informationType)
  {
    return !SpecificImageExists(caseID, informationType, controlPoint);
  };

  allInformationTypes.erase(std::remove_if(allInformationTypes.begin(), allInformationTypes.end(), lambda), allInformationTypes.end());

  return allInformationTypes;
}

bool mitk::SemanticRelationsInference::InstanceExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType)
{
  SemanticTypes::InformationTypeVector allInformationTypes = RelationStorage::GetAllInformationTypesOfCase(caseID);

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

bool mitk::SemanticRelationsInference::SpecificImageExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::InformationType& informationType)
{
  SemanticTypes::IDVector allImageIDsOfLesion;
  try
  {
    allImageIDsOfLesion = GetAllImageIDsOfLesion(caseID, lesion);
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }

  SemanticTypes::IDVector allImageIDsOfInformationType = RelationStorage::GetAllImageIDsOfInformationType(caseID, informationType);

  std::sort(allImageIDsOfLesion.begin(), allImageIDsOfLesion.end());
  std::sort(allImageIDsOfInformationType.begin(), allImageIDsOfInformationType.end());
  SemanticTypes::IDVector allImageIDsIntersection;
  // set_intersection removes duplicated nodes, since 'GetAllImageIDsOfInformationType' only contains at most one of each node
  std::set_intersection(allImageIDsOfLesion.begin(), allImageIDsOfLesion.end(),
    allImageIDsOfInformationType.begin(), allImageIDsOfInformationType.end(),
    std::back_inserter(allImageIDsIntersection));

  // if the vector of intersecting image IDs is empty, the information type does not contain the lesion
  return !allImageIDsIntersection.empty();
}

bool mitk::SemanticRelationsInference::SpecificImageExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::IDVector allImageIDsOfLesion;
  try
  {
    allImageIDsOfLesion = GetAllImageIDsOfLesion(caseID, lesion);
  }
  catch (const SemanticRelationException&)
  {
    return false;
  }

  SemanticTypes::IDVector allImageIDsOfControlPoint = RelationStorage::GetAllImageIDsOfControlPoint(caseID, controlPoint);

  std::sort(allImageIDsOfLesion.begin(), allImageIDsOfLesion.end());
  std::sort(allImageIDsOfControlPoint.begin(), allImageIDsOfControlPoint.end());
  SemanticTypes::IDVector allImageIDsIntersection;
  // set_intersection removes duplicated nodes, since 'GetAllImageIDsOfControlPoint' only contains at most one of each node
  std::set_intersection(allImageIDsOfLesion.begin(), allImageIDsOfLesion.end(),
    allImageIDsOfControlPoint.begin(), allImageIDsOfControlPoint.end(),
    std::back_inserter(allImageIDsIntersection));

  // if the vector of intersecting image IDs is empty, the control point does not contain the lesion
  return !allImageIDsIntersection.empty();
}

bool mitk::SemanticRelationsInference::SpecificImageExists(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::IDVector allImageIDsOfInformationType = RelationStorage::GetAllImageIDsOfInformationType(caseID, informationType);
  SemanticTypes::IDVector allImageIDsOfControlPoint = RelationStorage::GetAllImageIDsOfControlPoint(caseID, controlPoint);

  std::sort(allImageIDsOfInformationType.begin(), allImageIDsOfInformationType.end());
  std::sort(allImageIDsOfControlPoint.begin(), allImageIDsOfControlPoint.end());
  SemanticTypes::IDVector allImageIDsIntersection;
  // set_intersection removes duplicated nodes
  std::set_intersection(allImageIDsOfInformationType.begin(), allImageIDsOfInformationType.end(),
    allImageIDsOfControlPoint.begin(), allImageIDsOfControlPoint.end(),
    std::back_inserter(allImageIDsIntersection));

  // if the vector of intersecting image IDs is empty no image exists for the given information type and control point
  return !allImageIDsIntersection.empty();
}
