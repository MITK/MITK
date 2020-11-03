/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations module
#include "mitkDICOMHelper.h"
#include "mitkSemanticRelationException.h"
#include "mitkUIDGeneratorBoost.h"

// mitk core
#include <mitkPropertyNameHelper.h>

// c++
#include <algorithm>

mitk::SemanticTypes::ControlPoint GetControlPointFromString(const std::string& dateAsString)
{
  // date expected to be YYYYMMDD (8 characters)
  if (dateAsString.size() != 8)
  {
    // string does not represent a DICOM date
    mitkThrowException(mitk::SemanticRelationException) << "Not a valid DICOM date format.";
  }

  mitk::SemanticTypes::ControlPoint controlPoint;
  controlPoint.SetDateFromString(dateAsString);

  return controlPoint;
}

std::string mitk::GetCaseIDDICOMProperty()
{
  // extract suitable DICOM tag to use as the case id
  // two alternatives can be used:
  //        - DICOM tag "0x0010, 0x0010" is PatientName
  //        - DICOM tag "0x0010, 0x0020" is PatientID
  // in the current implementation the PatientName (0x0010, 0x0010) is used
  return GeneratePropertyNameForDICOMTag(0x0010, 0x0010);
}

std::string mitk::GetNodeIDDICOMProperty()
{
  // extract suitable DICOM tag to use as the data node id
  // DICOM tag "0x0020, 0x000e" is SeriesInstanceUID
  return GeneratePropertyNameForDICOMTag(0x0020, 0x000e);
}

std::string mitk::GetDateDICOMProperty()
{
  // extract suitable DICOM tag to use as the data node id
  // DICOM tag "0x0008, 0x0022" is AcquisitionDate
  return GeneratePropertyNameForDICOMTag(0x0008, 0x0022);
}

std::string mitk::GetModalityDICOMProperty()
{
  // extract suitable DICOM tag to use as the information type
  // DICOM tag "0x0008, 0x0060" is Modality
  return GeneratePropertyNameForDICOMTag(0x0008, 0x0060);
}

mitk::SemanticTypes::CaseID mitk::GetCaseIDFromDataNode(const DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    mitkThrowException(SemanticRelationException) << "No valid base data.";
  }

  BaseProperty* dicomTag = baseData->GetProperty(GetCaseIDDICOMProperty().c_str());
  if (nullptr == dicomTag)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid DICOM property.";
  }

  std::string dicomTagAsString = dicomTag->GetValueAsString();
  return dicomTagAsString;
}

mitk::SemanticTypes::ID mitk::GetIDFromDataNode(const DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    mitkThrowException(SemanticRelationException) << "No valid base data.";
  }

  BaseProperty* dicomTag = baseData->GetProperty(GetNodeIDDICOMProperty().c_str());
  if (nullptr == dicomTag)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid DICOM property.";
  }
  std::string dicomTagAsString = dicomTag->GetValueAsString();
  return dicomTagAsString;
}

mitk::SemanticTypes::ControlPoint mitk::GetDICOMDateFromDataNode(const DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    mitkThrowException(SemanticRelationException) << "No valid base data.";
  }

  BaseProperty* acquisitionDateProperty = baseData->GetProperty(GetDateDICOMProperty().c_str());
  if (nullptr == acquisitionDateProperty)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid DICOM property.";
  }
  std::string acquisitionDateAsString = acquisitionDateProperty->GetValueAsString();

  SemanticTypes::ControlPoint controlPoint;
  try
  {
    controlPoint = GetControlPointFromString(acquisitionDateAsString);
  }
  catch (SemanticRelationException &e)
  {
    mitkReThrow(e) << "Cannot retrieve a valid DICOM date.";
  }

  return controlPoint;
}

mitk::SemanticTypes::InformationType mitk::GetDICOMModalityFromDataNode(const DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid data node.";
  }

  BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    mitkThrowException(SemanticRelationException) << "No valid base data.";
  }

  BaseProperty* dicomTag = baseData->GetProperty(GetModalityDICOMProperty().c_str());
  if (nullptr == dicomTag)
  {
    mitkThrowException(SemanticRelationException) << "Not a valid DICOM property.";
  }
  std::string dicomTagAsString = dicomTag->GetValueAsString();
  return dicomTagAsString;
}

std::string mitk::TrimDICOM(const std::string& identifier)
{
  if (identifier.empty())
  {
    return identifier;
  }

  // leading whitespace
  std::size_t first = identifier.find_first_not_of(' ');
  if (std::string::npos == first)
  {
    return "";
  }
  // trailing whitespace
  std::size_t last = identifier.find_last_not_of(' ');
  return identifier.substr(first, last - first + 1);
}
