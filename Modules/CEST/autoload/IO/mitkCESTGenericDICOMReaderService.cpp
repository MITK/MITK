/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTGenericDICOMReaderService.h"

#include "mitkIOMimeTypes.h"
#include <mitkCustomTagParser.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderSelector.h>
#include "mitkCESTImageNormalizationFilter.h"

#include "itksys/SystemTools.hxx"

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace mitk
{
  CESTDICOMManualReaderService::CESTDICOMManualReaderService(const CustomMimeType& mimeType, const std::string& description)
    : BaseDICOMReaderService(mimeType, description)
  {
    IFileIO::Options options;
    options["B1 amplitude"] = 0.0;
    options["CEST frequency [Hz]"] = 0.0;
    options["Pulse duration [us]"] = 0.0;
    options["Duty cycle [%]"] = 0.0;
    std::vector<std::string> normalizationStrategy;
    normalizationStrategy.push_back("Automatic");
    normalizationStrategy.push_back("No");
    options["Normalize data"] = normalizationStrategy;

    this->SetDefaultOptions(options);

    this->RegisterService();
  }

  CESTDICOMManualReaderService::CESTDICOMManualReaderService(const mitk::CESTDICOMManualReaderService& other)
    : BaseDICOMReaderService(other)
  {
  }

  void ExtractOptionFromPropertyTree(const std::string& key, boost::property_tree::ptree& root, std::map<std::string, us::Any>& options)
  {
    auto finding = root.find(key);
    if (finding != root.not_found())
    {
      options[key] = finding->second.get_value<double>();
    }
  }

  IFileIO::Options ExtractOptionsFromFile(const std::string& file)
  {
    boost::property_tree::ptree root;

    try
    {
      boost::property_tree::read_json(file, root, std::locale("C"));
    }
    catch (const boost::property_tree::json_parser_error & e)
    {
      MITK_WARN << "Could not parse CEST meta file. Fall back to default values. Error was:\n" << e.what();
    }

    IFileIO::Options options;
    ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_FREQ(),root, options);
    ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_B1Amplitude(), root, options);
    ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_PULSEDURATION(), root, options);
    ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_DutyCycle(), root, options);
    ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_OFFSETS(), root, options);
    ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_TREC(), root, options);

    return options;
  }

  void TransferOption(const mitk::IFileIO::Options& sourceOptions, const std::string& sourceName, mitk::IFileIO::Options& options, const std::string& newName)
  {
    auto sourceFinding = sourceOptions.find(sourceName);
    auto finding = options.find(newName);

    bool replaceValue = finding == options.end();
    if (!replaceValue)
    {
      replaceValue = us::any_cast<double>(finding->second) == 0.;
    }

    if (sourceFinding != sourceOptions.end() && us::any_cast<double>(sourceFinding->second) != 0. && replaceValue)
    {
      options[newName] = sourceFinding->second;
    }
  }
  
  std::string CESTDICOMManualReaderService::GetCESTMetaFilePath() const
  {
    auto dir = itksys::SystemTools::GetFilenamePath(this->GetInputLocation());
    std::string metafile = dir + "/" + "CEST_META.json";
    return metafile;
  }

  std::string CESTDICOMManualReaderService::GetTRECFilePath() const
  {
    auto dir = itksys::SystemTools::GetFilenamePath(this->GetInputLocation());
    std::string metafile = dir + "/" + "TREC.txt";
    return metafile;
  }

  std::string CESTDICOMManualReaderService::GetLISTFilePath() const
  {
    auto dir = itksys::SystemTools::GetFilenamePath(this->GetInputLocation());
    std::string metafile = dir + "/" + "LIST.txt";
    return metafile;
  }


  IFileIO::Options CESTDICOMManualReaderService::GetOptions() const
  {
    auto options = AbstractFileReader::GetOptions();
    if (!this->GetInputLocation().empty())
    {
      auto fileOptions = ExtractOptionsFromFile(this->GetCESTMetaFilePath());

      TransferOption(fileOptions, CEST_PROPERTY_NAME_FREQ(), options, "CEST frequency [Hz]");
      TransferOption(fileOptions, CEST_PROPERTY_NAME_B1Amplitude(), options, "B1 amplitude");
      TransferOption(fileOptions, CEST_PROPERTY_NAME_PULSEDURATION(), options, "Pulse duration [us]");
      TransferOption(fileOptions, CEST_PROPERTY_NAME_DutyCycle(), options, "Duty cycle [%]");
    }
    return options;
  }

  us::Any CESTDICOMManualReaderService::GetOption(const std::string& name) const
  {
    this->GetOptions(); //ensure (default) options are set.
    return AbstractFileReader::GetOption(name);
  }

  DICOMFileReader::Pointer CESTDICOMManualReaderService::GetReader(const mitk::StringList& relevantFiles) const
  {
    mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

    selector->LoadBuiltIn3DnTConfigs();
    selector->SetInputFiles(relevantFiles);

    mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
    if (reader.IsNotNull())
    {
      //reset tag cache to ensure that additional tags of interest
      //will be regarded by the reader if set later on.
      reader->SetTagCache(nullptr);
    }

    return reader;
  };

  std::vector<itk::SmartPointer<BaseData>> CESTDICOMManualReaderService::Read()
  {
    std::vector<BaseData::Pointer> result;
    std::vector<BaseData::Pointer> dicomResult = BaseDICOMReaderService::Read();

    const Options userOptions = this->GetOptions();

    const std::string normalizationStrategy = userOptions.find("Normalize data")->second.ToString();

    for (auto &item : dicomResult)
    {
      auto fileOptions = ExtractOptionsFromFile(this->GetCESTMetaFilePath());
      IFileIO::Options options;
      TransferOption(userOptions, "CEST frequency [Hz]", options, CEST_PROPERTY_NAME_FREQ());
      TransferOption(userOptions, "B1 amplitude", options, CEST_PROPERTY_NAME_B1Amplitude());
      TransferOption(userOptions, "Pulse duration [us]", options, CEST_PROPERTY_NAME_PULSEDURATION());
      TransferOption(userOptions, "Duty cycle [%]", options, CEST_PROPERTY_NAME_DutyCycle());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_FREQ(), options, CEST_PROPERTY_NAME_FREQ());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_B1Amplitude(), options, CEST_PROPERTY_NAME_B1Amplitude());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_PULSEDURATION(), options, CEST_PROPERTY_NAME_PULSEDURATION());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_DutyCycle(), options, CEST_PROPERTY_NAME_DutyCycle());

      TransferOption(fileOptions, CEST_PROPERTY_NAME_OFFSETS(), options, CEST_PROPERTY_NAME_OFFSETS());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_TREC(), options, CEST_PROPERTY_NAME_TREC());

      auto trecValues = CustomTagParser::ReadListFromFile(this->GetTRECFilePath());
      auto offsetValues = CustomTagParser::ReadListFromFile(this->GetLISTFilePath());

      bool isCEST = !offsetValues.empty();
      bool isT1 = !trecValues.empty();

      if (!isCEST && !isT1)
      {//check if there are settings in the metafile
        auto finding = fileOptions.find(CEST_PROPERTY_NAME_OFFSETS());
        if (finding != fileOptions.end())
        {
          isCEST = true;
          offsetValues = finding->second.ToString();
        };

        finding = fileOptions.find(CEST_PROPERTY_NAME_TREC());
        if (finding != fileOptions.end())
        {
          isT1 = true;
          trecValues = finding->second.ToString();
        };
      }

      if (isCEST)
      {
        MITK_INFO << "CEST image detected due to LIST.txt or offset property in CEST_META.json";
        options[CEST_PROPERTY_NAME_OFFSETS()] = offsetValues;
      }
      else if (isT1)
      {
        MITK_INFO << "T1 image detected due to TREC.txt or trec property in CEST_META.json";
        options[CEST_PROPERTY_NAME_TREC()] = trecValues;
      }
      else
      {
        mitkThrow() << "Cannot load CEST/T1 file. No CEST offsets or T1 trec values specified. LIST.txt/TREC.txt or information in CEST_META.json is missing.";
      }

      for (const auto& option : options)
      {
        item->GetPropertyList()->SetStringProperty(option.first.c_str(), option.second.ToString().c_str());
      }

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

  CESTDICOMManualReaderService *CESTDICOMManualReaderService::Clone() const { return new CESTDICOMManualReaderService(*this); }
}
