/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations module
#include "mitkLesionManager.h"
#include "mitkRelationStorage.h"
#include "mitkSemanticRelationException.h"
#include "mitkSemanticRelationsInference.h"
#include "mitkUIDGeneratorBoost.h"

mitk::SemanticTypes::Lesion mitk::GenerateNewLesion(const std::string& lesionClassType/* = ""*/)
{
  SemanticTypes::Lesion lesion;
  lesion.UID = UIDGeneratorBoost::GenerateUID();
  lesion.name = "New lesion";
  lesion.lesionClass = GenerateNewLesionClass(lesionClassType);

  return lesion;
}

mitk::SemanticTypes::LesionClass mitk::GenerateNewLesionClass(const std::string& lesionClassType/* = ""*/)
{
  SemanticTypes::LesionClass lesionClass;
  lesionClass.UID = UIDGeneratorBoost::GenerateUID();
  lesionClass.classType = lesionClassType;

  return lesionClass;
}

mitk::SemanticTypes::Lesion mitk::GetLesionByUID(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& lesionUID)
{
  auto lambda = [&lesionUID](const SemanticTypes::Lesion& currentLesion)
  {
    return currentLesion.UID == lesionUID;
  };

  SemanticTypes::LesionVector allLesions = RelationStorage::GetAllLesionsOfCase(caseID);
  const auto existingLesion = std::find_if(allLesions.begin(), allLesions.end(), lambda);

  SemanticTypes::Lesion lesion;
  if (existingLesion != allLesions.end())
  {
    lesion = *existingLesion;
  }

  return lesion;
}

mitk::SemanticTypes::LesionClass mitk::FindExistingLesionClass(const SemanticTypes::CaseID& caseID, const std::string& lesionClassType)
{
  auto lambda = [&lesionClassType](const SemanticTypes::LesionClass& currentLesionClass)
  {
    return currentLesionClass.classType == lesionClassType;
  };

  SemanticTypes::LesionClassVector allLesionClasses = SemanticRelationsInference::GetAllLesionClassesOfCase(caseID);
  const auto existingLesionClass = std::find_if(allLesionClasses.begin(), allLesionClasses.end(), lambda);

  SemanticTypes::LesionClass lesionClass;
  if (existingLesionClass != allLesionClasses.end())
  {
    lesionClass = *existingLesionClass;
  }

  return lesionClass;
}

void mitk::ComputeLesionPresence(LesionData& lesionData, const SemanticTypes::CaseID& caseID)
{
  std::vector<bool> lesionPresence;
  auto lesion = lesionData.GetLesion();
  bool presence = false;
  auto controlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);
  // sort the vector of control points for the timeline
  std::sort(controlPoints.begin(), controlPoints.end());
  for (const auto& controlPoint : controlPoints)
  {
    try
    {
      presence = SemanticRelationsInference::IsLesionPresentAtControlPoint(caseID, lesion, controlPoint);
    }
    catch (SemanticRelationException&)
    {
      presence = false;
    }

    lesionPresence.push_back(presence);
  }

  lesionData.SetLesionPresence(lesionPresence);
}
