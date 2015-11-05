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

#include "mitkAutoSelectingDICOMReaderService.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkImage.h>
#include <mitkDICOMFilesHelper.h>

#include <iostream>

namespace mitk {

AutoSelectingDICOMReaderService::AutoSelectingDICOMReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), "MITK DICOM Reader (auto)")
{
  this->RegisterService();
}

std::vector<itk::SmartPointer<BaseData> > AutoSelectingDICOMReaderService::Read()
{
  std::vector<BaseData::Pointer> result;
  std::string fileName = this->GetLocalFileName();

  mitk::StringList relevantFiles = mitk::GetDICOMFilesInSameDirectory(fileName);

  mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

  selector->LoadBuiltIn3DConfigs();
  selector->LoadBuiltIn3DnTConfigs();
  selector->SetInputFiles(relevantFiles);

  mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();

  reader->SetInputFiles(relevantFiles);
  reader->AnalyzeInputFiles();
  reader->LoadImages();

  for (unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
  {
    const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(i);
    mitk::BaseData::Pointer data = desc.GetMitkImage();

    std::string nodeName = "Unamed_DICOM";

    std::string studyDescription = desc.GetPropertyAsString("studyDescription");
    std::string seriesDescription = desc.GetPropertyAsString("seriesDescription");

    if (!studyDescription.empty())
    {
      nodeName = studyDescription;
    }

    if (!seriesDescription.empty())
    {
      if (!studyDescription.empty())
      {
        nodeName += "/";
      }
      nodeName += seriesDescription;
    }

    StringProperty::Pointer nameProp = StringProperty::New(nodeName);
    data->SetProperty("name", nameProp);

    result.push_back(data);
  }

  return result;
}

AutoSelectingDICOMReaderService* AutoSelectingDICOMReaderService::Clone() const
{
  return new AutoSelectingDICOMReaderService(*this);
}

}
