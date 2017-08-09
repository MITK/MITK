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

#include "mitkCESTDICOMReaderService.h"

#include "mitkCESTIOMimeTypes.h"
#include <mitkCustomTagParser.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderSelector.h>

#include <itkGDCMImageIO.h>

namespace mitk
{
  CESTDICOMReaderService::CESTDICOMReaderService()
    : BaseDICOMReaderService(CustomMimeType(MitkCESTIOMimeTypes::CEST_DICOM_MIMETYPE_NAME()), "MITK CEST DICOM Reader")
  {
    this->RegisterService();
  }

  DICOMFileReader::Pointer CESTDICOMReaderService::GetReader(const mitk::StringList &relevantFiles) const
  {
    mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

    selector->LoadBuiltIn3DConfigs();
    selector->LoadBuiltIn3DnTConfigs();
    selector->SetInputFiles(relevantFiles);

    mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
    if (reader.IsNotNull())
    {
      // reset tag cache to ensure that additional tags of interest
      // will be regarded by the reader if set later on.
      reader->SetTagCache(nullptr);
    }

    return reader;
  }

  std::vector<itk::SmartPointer<BaseData>> CESTDICOMReaderService::Read()
  {
    std::vector<BaseData::Pointer> result = BaseDICOMReaderService::Read();

    mitk::StringList relevantFiles = this->GetRelevantFiles();

    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();

    DICOMTag siemensCESTprivateTag(0x0029, 0x1020);

    scanner->AddTag(siemensCESTprivateTag);
    scanner->SetInputFiles(relevantFiles);
    scanner->Scan();
    mitk::DICOMTagCache::Pointer tagCache = scanner->GetScanCache();

    DICOMImageFrameList imageFrameList = mitk::ConvertToDICOMImageFrameList(tagCache->GetFrameInfoList());
    DICOMImageFrameInfo *firstFrame = imageFrameList.begin()->GetPointer();

    std::string byteString = tagCache->GetTagValue(firstFrame, siemensCESTprivateTag).value;

    mitk::CustomTagParser tagParser(relevantFiles[0]);

    auto parsedPropertyList = tagParser.ParseDicomPropertyString(byteString);

    for (auto &item : result)
    {
      item->GetPropertyList()->ConcatenatePropertyList(parsedPropertyList);
    }

    return result;
  }

  CESTDICOMReaderService *CESTDICOMReaderService::Clone() const { return new CESTDICOMReaderService(*this); }
}
