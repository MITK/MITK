/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkModelFitCmdAppsHelper.h>


std::string sanitizeString(const std::string& path)
{
  auto result = path;
  std::string illegalChars = "\\/:?\"<>|%* ";
  for (auto &c : result)
    {
      bool found = illegalChars.find(c) != std::string::npos;
      if (found){
        c = '_';
      }
    }
  return result;
}

MITKMODELFIT_EXPORT std::string mitk::generateModelFitResultImagePath(const std::string& outputPathTemplate, const std::string& parameterName)
{
  std::string ext = ::itksys::SystemTools::GetFilenameLastExtension(outputPathTemplate);

  std::string dir = itksys::SystemTools::GetFilenamePath(outputPathTemplate);
  dir = itksys::SystemTools::ConvertToOutputPath(dir);

  std::string rootName = itksys::SystemTools::GetFilenameWithoutLastExtension(outputPathTemplate);

  std::string fileName = rootName + "_" + sanitizeString(parameterName) + ext;

  std::string fullOutPath = itksys::SystemTools::ConvertToOutputPath(dir + "/" + fileName);

  return fullOutPath;
}

std::string convertParameterTypeToStr(mitk::modelFit::Parameter::Type type)
{
  switch (type)
  {
  case mitk::modelFit::Parameter::ParameterType: return "parameter";
  case mitk::modelFit::Parameter::DerivedType: return "derived";
  case mitk::modelFit::Parameter::CriterionType: return "criterion";
  case mitk::modelFit::Parameter::EvaluationType: return "evaluation";
  default: return "unkown";
  }
}

MITKMODELFIT_EXPORT void mitk::storeModelFitResultImage(const std::string& outputPathTemplate, const std::string& parameterName, mitk::Image* image, mitk::modelFit::Parameter::Type nodeType, const mitk::modelFit::ModelFitInfo* modelFitInfo)
{
  mitk::modelFit::SetModelFitDataProperties(image, parameterName, nodeType, modelFitInfo);

  storeParameterResultImage(outputPathTemplate, parameterName, image, nodeType);
}

MITKMODELFIT_EXPORT void mitk::storeParameterResultImage(const std::string& outputPathTemplate, const std::string& parameterName, mitk::Image* image, mitk::modelFit::Parameter::Type parameterType)
{
  std::string fullOutPath = generateModelFitResultImagePath(outputPathTemplate, parameterName);

  mitk::IOUtil::Save(image, fullOutPath);

  std::cout << "Store result " << convertParameterTypeToStr(parameterType) << ": " << parameterName << " -> " << fullOutPath << std::endl;
}

MITKMODELFIT_EXPORT void mitk::storeModelFitGeneratorResults(const std::string& outputPathTemplate, mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitSession)
{
  if (generator)
  {
    for (const auto &imageIterator : generator->GetParameterImages())
    {
      storeModelFitResultImage(outputPathTemplate, imageIterator.first, imageIterator.second, mitk::modelFit::Parameter::ParameterType, fitSession);
    }
    for (const auto &imageIterator : generator->GetDerivedParameterImages())
    {
      storeModelFitResultImage(outputPathTemplate, imageIterator.first, imageIterator.second, mitk::modelFit::Parameter::DerivedType, fitSession);
    }
    for (const auto &imageIterator : generator->GetCriterionImages())
    {
      storeModelFitResultImage(outputPathTemplate, imageIterator.first, imageIterator.second, mitk::modelFit::Parameter::CriterionType, fitSession);
    }
    for (const auto &imageIterator : generator->GetEvaluationParameterImages())
    {
      storeModelFitResultImage(outputPathTemplate, imageIterator.first, imageIterator.second, mitk::modelFit::Parameter::EvaluationType, fitSession);
    }
  }
}


MITKMODELFIT_EXPORT void mitk::previewModelFitGeneratorResults(const std::string& outputPathTemplate, mitk::ParameterFitImageGeneratorBase* generator)
{
  if (generator)
  {
    for (const auto &aName : generator->GetParameterNames())
    {
      std::cout << "Store result " << convertParameterTypeToStr(modelFit::Parameter::ParameterType) << ": " << aName << " -> " << generateModelFitResultImagePath(outputPathTemplate, aName) << std::endl;
    }
    for (const auto &aName : generator->GetDerivedParameterNames())
    {
      std::cout << "Store result " << convertParameterTypeToStr(modelFit::Parameter::DerivedType) << ": " << aName << " -> " << generateModelFitResultImagePath(outputPathTemplate, aName) << std::endl;
    }
    for (const auto &aName : generator->GetCriterionNames())
    {
      std::cout << "Store result " << convertParameterTypeToStr(modelFit::Parameter::CriterionType) << ": " << aName << " -> " << generateModelFitResultImagePath(outputPathTemplate, aName) << std::endl;
    }
    for (const auto &aName : generator->GetEvaluationParameterNames())
    {
      std::cout << "Store result " << convertParameterTypeToStr(modelFit::Parameter::EvaluationType) << ": " << aName << " -> " << generateModelFitResultImagePath(outputPathTemplate, aName) << std::endl;
    }
    std::cout << "Preview done." << std::endl;
  }
}
