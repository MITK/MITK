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
#include "mitkNodeIdentifier.h"

// core
#include <mitkPropertyNameHelper.h>

SemanticTypes::CaseID NodeIdentifier::GetCaseIDFromData(const mitk::DataNode* dataNode)
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

SemanticTypes::ID NodeIdentifier::GetIDFromData(const mitk::DataNode* dataNode)
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
