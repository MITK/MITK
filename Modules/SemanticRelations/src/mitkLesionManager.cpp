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
#include "mitkSemanticRelationException.h"
#include "mitkUIDGeneratorBoost.h"

bool GetLesionPresence(const mitk::SemanticTypes::CaseID& caseID, std::shared_ptr<mitk::SemanticRelations> semanticRelations, const mitk::SemanticTypes::Lesion& lesion, const mitk::SemanticTypes::ControlPoint& controlPoint);

double GetLesionVolume(const mitk::SemanticTypes::CaseID& caseID, std::shared_ptr<mitk::SemanticRelations> semanticRelations, const mitk::SemanticTypes::Lesion& lesion, const mitk::SemanticTypes::ControlPoint& controlPoint);

mitk::SemanticTypes::Lesion mitk::GenerateNewLesion(const std::string& lesionClassType/* = ""*/)
{
  mitk::SemanticTypes::Lesion lesion;
  lesion.UID = mitk::UIDGeneratorBoost::GenerateUID();
  lesion.name = "New lesion";
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

void mitk::GenerateAdditionalLesionData(LesionData& lesionData, const SemanticTypes::CaseID& caseID, std::shared_ptr<SemanticRelations> semanticRelations)
{
  std::vector<bool> lesionPresence;
  std::vector<double> lesionVolume;
  SemanticTypes::Lesion lesion = lesionData.GetLesion();
  bool presence = false;
  double volume = 0.0;
  try
  {
    std::vector<SemanticTypes::ControlPoint> controlPoints = semanticRelations->GetAllControlPointsOfCase(caseID);
    for (const auto& controlPoint : controlPoints)
    {
      presence = GetLesionPresence(caseID, semanticRelations, lesion, controlPoint);
      lesionPresence.push_back(presence);

      volume = GetLesionVolume(caseID, semanticRelations, lesion, controlPoint);
      lesionVolume.push_back(volume);
    }
  }
  catch (const mitk::SemanticRelationException&)
  {
    return;
  }

  lesionData.SetLesionPresence(lesionPresence);
  lesionData.SetLesionVolume(lesionVolume);
}

bool GetLesionPresence(const mitk::SemanticTypes::CaseID& caseID, std::shared_ptr<mitk::SemanticRelations> semanticRelations, const mitk::SemanticTypes::Lesion& lesion, const mitk::SemanticTypes::ControlPoint& controllPoint)
{
  try
  {
    mitk::SemanticRelations::DataNodeVector allImagesOfLesion = semanticRelations->GetAllImagesOfLesion(caseID, lesion);
    for (const auto& image : allImagesOfLesion)
    {
      auto imageControlPoint = semanticRelations->GetControlPointOfImage(image);
      if (imageControlPoint == controllPoint)
      {
        return true;
      }
    }
  }
  catch (const mitk::SemanticRelationException&)
  {
    return false;
  }

  return false;
}

double GetLesionVolume(const mitk::SemanticTypes::CaseID& caseID, std::shared_ptr<mitk::SemanticRelations> semanticRelations, const mitk::SemanticTypes::Lesion& lesion, const mitk::SemanticTypes::ControlPoint& controlPoint)
{
  bool presence = GetLesionPresence(caseID, semanticRelations, lesion, controlPoint);
  if (presence)
  {
    return 1.0;
  }
  else
  {
    return 0.0;
  }
}
