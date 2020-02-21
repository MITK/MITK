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

#include "mitkBaseDICOMReaderService.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkImage.h>
#include <mitkDICOMFilesHelper.h>
#include <mitkDICOMTagHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkDicomSeriesReader.h>
#include <mitkLocaleSwitch.h>
#include <iostream>

namespace mitk {

  BaseDICOMReaderService::BaseDICOMReaderService(const std::string& description)
    : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), description)
{
}

std::vector<itk::SmartPointer<BaseData> > BaseDICOMReaderService::Read()
{
  std::vector<BaseData::Pointer> result;

  mitk::StringList relevantFiles = this->GetRelevantFiles();

  mitk::DICOMFileReader::Pointer reader = this->GetReader(relevantFiles);

  reader->SetAdditionalTagsOfInterest(mitk::GetCurrentDICOMTagsOfInterest());
  reader->SetTagLookupTableToPropertyFunctor(mitk::GetDICOMPropertyForDICOMValuesFunctor);
  reader->SetInputFiles(relevantFiles);
  reader->AnalyzeInputFiles();
  reader->LoadImages();

  for (unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
  {
    const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(i);
    mitk::BaseData::Pointer data = desc.GetMitkImage().GetPointer();

    std::string nodeName = "Unnamed_DICOM";

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

StringList BaseDICOMReaderService::GetRelevantFiles() const
{
  std::string fileName = this->GetLocalFileName();

  mitk::StringList relevantFiles = mitk::GetDICOMFilesInSameDirectory(fileName);

  return relevantFiles;
}


}
