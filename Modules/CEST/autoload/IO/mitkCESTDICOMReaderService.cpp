/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTDICOMReaderService.h"

#include "mitkCESTIOMimeTypes.h"
#include <mitkCustomTagParser.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderSelector.h>
#include "mitkCESTImageNormalizationFilter.h"

#include <itkGDCMImageIO.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

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

    std::vector<std::string> normalizationStrategy;
    normalizationStrategy.push_back("Automatic");
    normalizationStrategy.push_back("No");
    defaultOptions["Normalize data"] = normalizationStrategy;


    this->SetDefaultOptions(defaultOptions);
    this->SetOnlyRegardOwnSeries(false);

    this->RegisterService();
  }

  DICOMFileReader::Pointer CESTDICOMReaderService::GetReader(const mitk::StringList &relevantFiles) const
  {
    mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

    auto r = ::us::GetModuleContext()->GetModule()->GetResource("cest_DKFZ.xml");
    selector->AddConfigFromResource(r);
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
    std::vector<BaseData::Pointer> result;
    std::vector<BaseData::Pointer> dicomResult = BaseDICOMReaderService::Read();

    const Options options = this->GetOptions();

    const std::string parseStrategy = options.find("Force type")->second.ToString();
    const std::string mappingStrategy = options.find("Revision mapping")->second.ToString();
    const std::string normalizationStrategy = options.find("Normalize data")->second.ToString();

    for (auto &item : dicomResult)
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

      auto image = dynamic_cast<mitk::Image*>(item.GetPointer());
      if (normalizationStrategy == "Automatic" && mitk::IsNotNormalizedCESTImage(image))
      {
        MITK_INFO << "Unnormalized CEST image was loaded and will be normalized automatically.";
        auto normalizationFilter = mitk::CESTImageNormalizationFilter::New();
        normalizationFilter->SetInput(image);
        normalizationFilter->Update();
        auto normalizedImage = normalizationFilter->GetOutput();

        auto nameProp = item->GetProperty("name");
        if (!nameProp)
        {
          mitkThrow() << "Cannot load CEST file. Property \"name\" is missing after BaseDICOMReaderService::Read().";
        }
        normalizedImage->SetProperty("name", mitk::StringProperty::New(nameProp->GetValueAsString() + "_normalized"));
        result.push_back(normalizedImage);
      }
      else
      {
        result.push_back(item);
      }
    }

    return result;
  }

  CESTDICOMReaderService *CESTDICOMReaderService::Clone() const { return new CESTDICOMReaderService(*this); }
}
