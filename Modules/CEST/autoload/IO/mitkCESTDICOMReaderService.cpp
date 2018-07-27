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
    Options defaultOptions;

    std::vector<std::string> parseStrategy;
    parseStrategy.push_back("Automatic");
    parseStrategy.push_back("CEST/WASABI");
    parseStrategy.push_back("T1");
    defaultOptions["Force type"] = parseStrategy;

    std::vector<std::string> mappingStrategy;
    mappingStrategy.push_back("Strict");
    mappingStrategy.push_back("Fuzzy");
    defaultOptions["Revision mapping"] = mappingStrategy;

    this->SetDefaultOptions(defaultOptions);

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

    const Options options = this->GetOptions();

    const std::string parseStrategy = options.find("Force type")->second.ToString();
    const std::string mappingStrategy = options.find("Revision mapping")->second.ToString();

    for (auto &item : result)
    {
      auto prop = item->GetProperty("files");
      auto fileProp = dynamic_cast<mitk::StringLookupTableProperty*>(prop.GetPointer());
      if (!fileProp)
      {
        mitkThrow() << "Cannot load CEST file. Property \"files\" is missing after BaseDICOMReaderService::Read().";
      }

      mitk::StringList relevantFiles = { fileProp->GetValue().GetTableValue(0) };

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
      tagParser.SetParseStrategy(parseStrategy);
      tagParser.SetRevisionMappingStrategy(mappingStrategy);

      auto parsedPropertyList = tagParser.ParseDicomPropertyString(byteString);

      item->GetPropertyList()->ConcatenatePropertyList(parsedPropertyList);
    }

    return result;
  }

  CESTDICOMReaderService *CESTDICOMReaderService::Clone() const { return new CESTDICOMReaderService(*this); }
}
