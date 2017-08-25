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

// semantic relation module
#include "mitkDICOMHelper.h"
#include "mitkUIDGeneratorBoost.h"

// mitk core
#include <mitkPropertyNameHelper.h>

// c++
#include <algorithm>

mitk::SemanticTypes::CaseID mitk::DICOMHelper::GetCaseIDFromDataNode(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return SemanticTypes::CaseID();
  }

  mitk::BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    MITK_INFO << "No valid base data.";
    return SemanticTypes::CaseID();
  }

  // extract suitable DICOM tag to use as the case id
  // DICOM tag "0x0010, 0x0010" is PatientName
  // DICOM tag "0x0010, 0x0020" is PatientID
  mitk::BaseProperty* dicomTag = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str());
  if (nullptr != dicomTag)
  {
    std::string dicomTagAsString = dicomTag->GetValueAsString();
    return dicomTagAsString;
  }
  return "";
}

mitk::SemanticTypes::ID mitk::DICOMHelper::GetIDFromDataNode(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return SemanticTypes::CaseID();
  }

  mitk::BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    MITK_INFO << "No valid base data.";
    return SemanticTypes::CaseID();
  }

  // extract suitable DICOM tag to use as the data node id
  // DICOM tag "0x0020, 0x000e" is SeriesInstanceUID
  mitk::BaseProperty* dicomTag = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str());
  if (nullptr != dicomTag)
  {
    std::string dicomTagAsString = dicomTag->GetValueAsString();
    return dicomTagAsString;
  }
  return "";
}

mitk::SemanticTypes::Date mitk::DICOMHelper::GetDateFromDataNode(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return SemanticTypes::Date();
  }

  mitk::BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    MITK_INFO << "No valid base data.";
    return SemanticTypes::Date();
  }

  // DICOM study date       = DICOMTag(0x0008, 0x0020)
  // DICOM series date      = DICOMTag(0x0008, 0x0021)
  // DICOM acquisition date = DICOMTag(0x0008, 0x0022)
  // => TODO: which to chose?
  mitk::BaseProperty* acquisitionDateProperty = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0022).c_str());
  if (nullptr != acquisitionDateProperty)
  {
    std::string acquisitionDateAsString = acquisitionDateProperty->GetValueAsString();
    return GetDateFromString(acquisitionDateAsString);
  }

  return SemanticTypes::Date();
}

void mitk::DICOMHelper::ReformatDICOMTag(const std::string &tag, std::string &identifier)
{
  // remove leading and trailing whitespace
  identifier = Trim(identifier);
  std::string tagName = DICOMTagToName(tag);
  if (tagName.length() >= 4)
  {
    if ("Date" == tagName.substr(tagName.length() - 4))
    {
      DICOMHelper::ReformatDICOMDate(identifier);
    }
    else if ("Time" == tagName.substr(tagName.length() - 4))
    {
      DICOMHelper::ReformatDICOMTime(identifier);
    }
  }
}

mitk::SemanticTypes::Date mitk::DICOMHelper::GetDateFromString(const std::string& dateAsString)
{
  if (dateAsString.size() != 8) // string does not represent a DICOM date
  {
    return SemanticTypes::Date();
  }

  SemanticTypes::Date date;
  date.UID = UIDGeneratorBoost::GenerateUID();
  // date expected to be YYYYMMDD (8 characters)
  date.year = std::strtoul(dateAsString.substr(0, 4).c_str(), nullptr, 10);
  date.month = std::strtoul(dateAsString.substr(4, 2).c_str(), nullptr, 10);
  date.day = std::strtoul(dateAsString.substr(6, 2).c_str(), nullptr, 10);

  return date;
}

std::string mitk::DICOMHelper::Trim(const std::string& identifier)
{
  if (identifier.empty())
  {
    return identifier;
  }

  std::size_t first = identifier.find_first_not_of(' ');
  if (std::string::npos == first)
  {
    return "";
  }
  std::size_t last = identifier.find_last_not_of(' ');
  return identifier.substr(first, last - first + 1);
}

std::string mitk::DICOMHelper::DICOMTagToName(const std::string& propertyName)
{
  mitk::DICOMTagPath tagPath = mitk::PropertyNameToDICOMTagPath(propertyName);
  return DICOMTagPathToName(tagPath);
}

std::string mitk::DICOMHelper::DICOMTagPathToName(const mitk::DICOMTagPath& tagPath)
{
  std::string tagName;
  int i = 0;
  for (const auto& node : tagPath.GetNodes())
  {
    if (i > 0)
    {
      tagName += "/";
    }

    tagName += node.tag.GetName();
    ++i;
  }
  return tagName;
}

void mitk::DICOMHelper::ReformatDICOMDate(std::string& identifier)
{
  if (identifier.size() != 8) // identifier does not represent a DICOM date
    return;

  // date expected to be YYYYMMDD (8 characters)
  // should output YYYY-MM-DD
  identifier.insert(4, 1, '-');
  identifier.insert(7, 1, '-');
}

void mitk::DICOMHelper::ReformatDICOMTime(std::string& identifier)
{
  if (identifier.size() != 6) // identifier does not represent a DICOM time
    return;

  // time expected to be HHMMSS (6 characters)
  // should output HH:MM:SS
  identifier.insert(2, 1, ':');
  identifier.insert(5, 1, ':');
}