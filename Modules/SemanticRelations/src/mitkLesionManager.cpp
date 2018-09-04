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
#include "mitkLesionManager.h"
#include "mitkUIDGeneratorBoost.h"

mitk::SemanticTypes::Lesion mitk::GenerateNewLesion(const std::string& lesionClassType/* = ""*/)
{
  mitk::SemanticTypes::Lesion lesion;
  lesion.UID = mitk::UIDGeneratorBoost::GenerateUID();
  lesion.name = lesion.UID;
  lesion.lesionClass = mitk::SemanticTypes::LesionClass();
  lesion.lesionClass.UID = mitk::UIDGeneratorBoost::GenerateUID();
  lesion.lesionClass.classType = lesionClassType;

  return lesion;
}

mitk::SemanticTypes::LesionClass mitk::GenerateNewLesionClass(const std::string& lesionClassType/* = ""*/)
{
  mitk::SemanticTypes::LesionClass lesionClass;
  lesionClass.UID = mitk::UIDGeneratorBoost::GenerateUID();
  lesionClass.classType = lesionClassType;

  return lesionClass;
}

mitk::SemanticTypes::Lesion mitk::GetLesionByUID(const SemanticTypes::ID& lesionUID, const std::vector<SemanticTypes::Lesion>& allLesions)
{
  auto lambda = [&lesionUID](const SemanticTypes::Lesion& currentLesion)
  {
    return currentLesion.UID == lesionUID;
  };

  const auto existingLesion = std::find_if(allLesions.begin(), allLesions.end(), lambda);

  mitk::SemanticTypes::Lesion lesion;
  if (existingLesion != allLesions.end())
  {
    lesion = *existingLesion;
  }

  return lesion;
}

mitk::SemanticTypes::LesionClass mitk::FindExistingLesionClass(const std::string& lesionClassType, const std::vector<SemanticTypes::LesionClass>& allLesionClasses)
{
  auto lambda = [&lesionClassType](const SemanticTypes::LesionClass& currentLesionClass)
  {
    return currentLesionClass.classType == lesionClassType;
  };

  const auto existingLesionClass = std::find_if(allLesionClasses.begin(), allLesionClasses.end(), lambda);

  mitk::SemanticTypes::LesionClass lesionClass;
  if (existingLesionClass != allLesionClasses.end())
  {
    lesionClass = *existingLesionClass;
  }

  return lesionClass;
}

void mitk::GenerateAdditionalLesionData(mitk::LesionData& lesionData, const SemanticTypes::CaseID& caseID, std::shared_ptr<SemanticRelations> semanticRelations)
{
  std::vector<bool> lesionPresence;
  std::vector<double> lesionVolume;
  mitk::SemanticTypes::Lesion lesion = lesionData.GetLesion();
  std::vector<mitk::SemanticTypes::ControlPoint> controlPoints = semanticRelations->GetAllControlPointsOfCase(caseID);
  for (const auto& controlPoint : controlPoints)
  {
    bool presence = semanticRelations->GetLesionPresence(caseID, lesion, controlPoint);
    lesionPresence.push_back(presence);

    double volume = semanticRelations->GetLesionVolume(caseID, lesion, controlPoint);
    lesionVolume.push_back(volume);
  }

  lesionData.SetLesionPresence(lesionPresence);
  lesionData.SetLesionVolume(lesionVolume);
}
