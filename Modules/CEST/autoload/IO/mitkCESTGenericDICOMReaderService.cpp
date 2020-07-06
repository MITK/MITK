/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTGenericDICOMReaderService.h"

#include "mitkIOMimeTypes.h"
#include <mitkExtractCESTOffset.h>
#include <mitkCustomTagParser.h>
#include <mitkCESTPropertyHelper.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMProperty.h>

#include "mitkCESTImageNormalizationFilter.h"

#include "itksys/SystemTools.hxx"

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace
{
  std::string OPTION_NAME_B1()
  {
    return "B1 amplitude";
  }

  std::string OPTION_NAME_PULSE()
  {
    return "Pulse duration [us]";
  }

  std::string OPTION_NAME_DC()
  {
    return "Duty cycle [%]";
  }

  std::string OPTION_NAME_NORMALIZE()
  {
    return "Normalize data";
  }

  std::string OPTION_NAME_NORMALIZE_AUTOMATIC()
  {
    return "Automatic";
  }

  std::string OPTION_NAME_NORMALIZE_NO()
  {
    return "No";
  }

  std::string OPTION_NAME_MERGE()
  {
    return "Merge all series";
  }

  std::string OPTION_NAME_MERGE_YES()
  {
    return "Yes";
  }

  std::string OPTION_NAME_MERGE_NO()
  {
    return "No";
  }

  std::string META_FILE_OPTION_NAME_MERGE()
  {
    return "CEST.MergeAllSeries";
  }

}

namespace mitk
{
  DICOMTagPath DICOM_IMAGING_FREQUENCY_PATH()
  {
    return mitk::DICOMTagPath(0x0018, 0x0084);
  }

  CESTDICOMManualReaderService::CESTDICOMManualReaderService(const CustomMimeType& mimeType, const std::string& description)
    : BaseDICOMReaderService(mimeType, description)
  {
    IFileIO::Options options;
    options[OPTION_NAME_B1()] = 0.0;
    options[OPTION_NAME_PULSE()] = 0.0;
    options[OPTION_NAME_DC()] = 0.0;
    std::vector<std::string> normalizationStrategy;
    normalizationStrategy.push_back(OPTION_NAME_NORMALIZE_AUTOMATIC());
    normalizationStrategy.push_back(OPTION_NAME_NORMALIZE_NO());
    options[OPTION_NAME_NORMALIZE()] = normalizationStrategy;
    std::vector<std::string> mergeStrategy;
    mergeStrategy.push_back(OPTION_NAME_MERGE_NO());
    mergeStrategy.push_back(OPTION_NAME_MERGE_YES());
    options[OPTION_NAME_MERGE()] = mergeStrategy;
    this->SetDefaultOptions(options);

    this->RegisterService();
  }

  namespace
  {
    void ExtractOptionFromPropertyTree(const std::string& key, boost::property_tree::ptree& root, std::map<std::string, us::Any>& options)
    {
      auto finding = root.find(key);
      if (finding != root.not_found())
      {
        try
        {
          options[key] = finding->second.get_value<double>();
        }
        catch (const boost::property_tree::ptree_bad_data& /*e*/)
        {
          options[key] = finding->second.get_value<std::string>();
        }
      }
    }

    IFileIO::Options ExtractOptionsFromFile(const std::string& file)
    {
      boost::property_tree::ptree root;

      if (itksys::SystemTools::FileExists(file))
      {
        try
        {
          boost::property_tree::read_json(file, root, std::locale("C"));
        }
        catch (const boost::property_tree::json_parser_error & e)
        {
          MITK_WARN << "Could not parse CEST meta file. Fall back to default values. Error was:\n" << e.what();
        }
      }
      else
      {
        MITK_DEBUG << "CEST meta file does not exist. Fall back to default values. CEST meta file path: " << file;
      }

      IFileIO::Options options;
      ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_B1Amplitude(), root, options);
      ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_PULSEDURATION(), root, options);
      ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_DutyCycle(), root, options);
      ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_OFFSETS(), root, options);
      ExtractOptionFromPropertyTree(CEST_PROPERTY_NAME_TREC(), root, options);
      ExtractOptionFromPropertyTree(META_FILE_OPTION_NAME_MERGE(), root, options);

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

    void TransferMergeOption(const mitk::IFileIO::Options& sourceOptions, const std::string& sourceName, mitk::IFileIO::Options& options, const std::string& newName)
    {
      auto sourceFinding = sourceOptions.find(sourceName);
      auto finding = options.find(newName);

      bool replaceValue = finding == options.end();
      if (!replaceValue)
      {
        try
        {
          us::any_cast<std::string>(finding->second);
        }
        catch (const us::BadAnyCastException& /*e*/)
        {
          replaceValue = true;
          //if we cannot cast in string the user has not made a selection yet
        }
      }

      if (sourceFinding != sourceOptions.end() && us::any_cast<std::string>(sourceFinding->second) != OPTION_NAME_MERGE_NO() && replaceValue)
      {
        options[newName] = sourceFinding->second;
      }
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

      TransferOption(fileOptions, CEST_PROPERTY_NAME_B1Amplitude(), options, OPTION_NAME_B1());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_PULSEDURATION(), options, OPTION_NAME_PULSE());
      TransferOption(fileOptions, CEST_PROPERTY_NAME_DutyCycle(), options, OPTION_NAME_DC());
      TransferMergeOption(fileOptions, META_FILE_OPTION_NAME_MERGE(), options, OPTION_NAME_MERGE());
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
    auto selector = mitk::DICOMFileReaderSelector::New();

    const std::string mergeStrategy = this->GetOption(OPTION_NAME_MERGE()).ToString();

    if (mergeStrategy == OPTION_NAME_MERGE_YES())
    {
      auto r = ::us::GetModuleContext()->GetModule()->GetResource("cest_DKFZ.xml");
      selector->AddConfigFromResource(r);
    }

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
  }

  std::vector<itk::SmartPointer<BaseData>> CESTDICOMManualReaderService::Read()
  {
    const Options userOptions = this->GetOptions();

    const std::string mergeStrategy = userOptions.find(OPTION_NAME_MERGE())->second.ToString();
    this->SetOnlyRegardOwnSeries(mergeStrategy != OPTION_NAME_MERGE_YES());

    std::vector<BaseData::Pointer> result;
    std::vector<BaseData::Pointer> dicomResult = BaseDICOMReaderService::Read();

    const std::string normalizationStrategy = userOptions.find(OPTION_NAME_NORMALIZE())->second.ToString();

    for (const auto &item : dicomResult)
    {
      auto fileOptions = ExtractOptionsFromFile(this->GetCESTMetaFilePath());
      IFileIO::Options options;
      TransferOption(userOptions, OPTION_NAME_B1(), options, CEST_PROPERTY_NAME_B1Amplitude());
      TransferOption(userOptions, OPTION_NAME_PULSE(), options, CEST_PROPERTY_NAME_PULSEDURATION());
      TransferOption(userOptions, OPTION_NAME_DC(), options, CEST_PROPERTY_NAME_DutyCycle());
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

      auto freqProp = item->GetProperty(mitk::DICOMTagPathToPropertyName(DICOM_IMAGING_FREQUENCY_PATH()).c_str());

      if (freqProp.IsNull())
      {
        mitkThrow() << "Loaded image in invalid state. Does not contain the DICOM Imaging Frequency tag.";
      }
      SetCESTFrequencyMHz(item, mitk::ConvertDICOMStrToValue<double>(freqProp->GetValueAsString()));

      auto image = dynamic_cast<mitk::Image*>(item.GetPointer());

      if (isCEST)
      {
        try
        {
          auto offsets = ExtractCESTOffset(image);
        }
        catch (...)
        {
          mitkThrow() << "Cannot load CEST file. Number of CEST offsets do not equal the number of image time steps. Image time steps: " << image->GetTimeSteps() << "; offset values: " << offsetValues;
        }
      }
      else if (isT1)
      {
        try
        {
          auto t1s = ExtractCESTT1Time(image);
        }
        catch (...)
        {
          mitkThrow() << "Cannot load T1 file. Number of T1 times do not equal the number of image time steps. Image time steps: " << image->GetTimeSteps() << "; T1 values: " << trecValues;
        }
      }

      if (normalizationStrategy == OPTION_NAME_NORMALIZE_AUTOMATIC() && mitk::IsNotNormalizedCESTImage(image))
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

  CESTDICOMManualReaderService *CESTDICOMManualReaderService::Clone() const
  {
    return new CESTDICOMManualReaderService(*this);
  }
}
