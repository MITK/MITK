/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationStackWriterBase.h"
#include "mitkImageAccessByItk.h"
#include "mitkMultiLabelIOHelper.h"
#include "mitkLabelSetImageConverter.h"
#include <mitkLocaleSwitch.h>
#include <mitkItkImageIO.h>

// itk
#include <itksys/SystemTools.hxx>

namespace
{
  constexpr const char* OPTION_SAVE_STRATEGY = "Save strategy";
  constexpr const char* OPTION_SAVE_STRATEGY_INSTANCE = "instance";
  constexpr const char* OPTION_SAVE_STRATEGY_GROUP = "group";
  constexpr const char* OPTION_INSTANCE_VALUE = "Instance value";
  constexpr const char* OPTION_INSTANCE_VALUE_BINARY = "binary";
  constexpr const char* OPTION_INSTANCE_VALUE_ORIGINAL = "original";
  constexpr int MULTILABEL_SEGMENTATION_VERSION_VALUE = 3;
}

namespace mitk
{
  MultiLabelSegmentationStackWriterBase::MultiLabelSegmentationStackWriterBase(const CustomMimeType& mimeType,
    const std::string& description)
    : AbstractFileWriter(MultiLabelSegmentation::GetStaticNameOfClass(), mimeType, description)
  {
    Options defaultOptions;

    std::vector<std::string> instanceSaveStrategy;
    instanceSaveStrategy.push_back(OPTION_SAVE_STRATEGY_GROUP);
    instanceSaveStrategy.push_back(OPTION_SAVE_STRATEGY_INSTANCE);
    defaultOptions[OPTION_SAVE_STRATEGY] = instanceSaveStrategy;
    std::vector<std::string> instanceValue;
    instanceValue.push_back(OPTION_INSTANCE_VALUE_ORIGINAL);
    instanceValue.push_back(OPTION_INSTANCE_VALUE_BINARY);
    defaultOptions[OPTION_INSTANCE_VALUE] = instanceValue;

    this->SetDefaultOptions(defaultOptions);
  }

  IFileIO::ConfidenceLevel MultiLabelSegmentationStackWriterBase::GetConfidenceLevel() const
  {
    if (AbstractFileWriter::GetConfidenceLevel() == Unsupported)
      return Unsupported;
    const auto *input = static_cast<const MultiLabelSegmentation *>(this->GetInput());
    if (input)
    {
      if (input->GetTimeGeometry()->CountTimeSteps() > 1)
      {
        //currently 4D is not supported. Should be fixed by issue #599
        return Unsupported;
      }

      return Supported;
    }
    else
    {
      return Unsupported;
    }
  }


  std::string GenerateGroupFilePath(const std::string& fileStemPath, const mitk::MultiLabelSegmentation::GroupIndexType& groupIndex, const std::string& fileExt)
  {
    return fileStemPath + "_group_" + std::to_string(groupIndex) + "" + fileExt;
  }
  std::string GenerateLabelFilePath(const std::string& fileStemPath, const mitk::MultiLabelSegmentation::LabelValueType& labelValue, const std::string& fileExt)
  {
    return fileStemPath + "_label_" + std::to_string(labelValue) + "" + fileExt;
  }

  void MultiLabelSegmentationStackWriterBase::Write()
  {
    ValidateOutputLocation();

    auto input = dynamic_cast<const MultiLabelSegmentation *>(this->GetInput());

    const auto options = this->GetOptions();

    const auto groupSaveStrategy = options.find(OPTION_SAVE_STRATEGY)->second.ToString() == OPTION_SAVE_STRATEGY_GROUP;
    const auto binaryValueMapping = options.find(OPTION_INSTANCE_VALUE)->second.ToString() == OPTION_INSTANCE_VALUE_BINARY;

    LocalFile localFile(this);
    const auto path = localFile.GetFileName();

    const auto imageFileExt = this->GetStackImageExtension();
    const auto absFileStemPath = itksys::SystemTools::JoinPath({"", itksys::SystemTools::GetFilenamePath(path), itksys::SystemTools::GetFilenameWithoutExtension(path)});
    const auto relFileStemPath = itksys::SystemTools::JoinPath({"", ".", itksys::SystemTools::GetFilenameWithoutExtension(path) });

    const auto saveStrategy = options.find(OPTION_SAVE_STRATEGY)->second.ToString();

    nlohmann::json metaInfo;

    if (groupSaveStrategy)
    {
      auto groupFileNameCallback = [relFileStemPath, imageFileExt](const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::GroupIndexType groupIndex)
        {
          return GenerateGroupFilePath(relFileStemPath, groupIndex, imageFileExt);
        };

      metaInfo = MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(input, groupFileNameCallback);

      auto imageIO = this->GetITKIO();
      // use compression if available
      imageIO->UseCompressionOn();
      mitk::LocaleSwitch localeSwitch("C");

      for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < input->GetNumberOfGroups(); ++groupID)
      {
        const auto groupImage = input->GetGroupImage(groupID);
        ItkImageIO::PreparImageIOToWriteImage(imageIO, groupImage);
        imageIO->SetFileName(GenerateGroupFilePath(absFileStemPath, groupID, imageFileExt));

        ImageReadAccessor imageAccess(groupImage);
        imageIO->Write(imageAccess.GetData());
      }
    }
    else
    {
      auto labelFileNameCallback = [relFileStemPath, imageFileExt](const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::LabelValueType labelValue)
        {
          return GenerateLabelFilePath(relFileStemPath, labelValue, imageFileExt);
        };

      auto labelFileValueCallback = [binaryValueMapping](const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::LabelValueType labelValue)
        {
          if (binaryValueMapping)
            return static_cast<mitk::MultiLabelSegmentation::LabelValueType>(1);

          return labelValue;
        };

      metaInfo = MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(input, nullptr, labelFileNameCallback, labelFileValueCallback);

      auto imageIO = this->GetITKIO();
      // use compression if available
      imageIO->UseCompressionOn();
      mitk::LocaleSwitch localeSwitch("C");

      for (const auto labelValue : input->GetAllLabelValues())
      {
        const auto labelImage = CreateLabelMask(input, labelValue, binaryValueMapping);
        ItkImageIO::PreparImageIOToWriteImage(imageIO, labelImage);
        imageIO->SetFileName(GenerateLabelFilePath(absFileStemPath, labelValue, imageFileExt));

        ImageReadAccessor imageAccess(labelImage);
        imageIO->Write(imageAccess.GetData());
      }
    }

    nlohmann::json stackContent;
    stackContent["version"] = MULTILABEL_SEGMENTATION_VERSION_VALUE;
    stackContent["type"] = "org.mitk.multilabel.segmentation.stack";

    //handle properties
    stackContent["groups"] = metaInfo;
    if (auto properties = input->GetPropertyList(); !properties->IsEmpty())
    {
      //clone properties to sort out the ones that should not be seriealized
      auto cleanedProps = properties->Clone();
      std::vector<std::string> unwantedPropNames = { "NRRD", "MITK.IO", "modality",
        "org.mitk.multilabel.segmentation", "org.mitk.uid", "path"};

      for (auto propName : cleanedProps->GetPropertyKeys())
      {
        for (auto unwantedName : unwantedPropNames)
        {
          // does the start match the default key
          if (propName.substr(0, unwantedName.length()).find(unwantedName) != std::string::npos)
          {
            cleanedProps->RemoveProperty(propName);
            break;
          }
        }
      }
      cleanedProps->ToJSON(stackContent["properties"]);

    }

    //handle UID
    stackContent["uid"] = input->GetUID();

    //save meta file
    const auto metaFileName = absFileStemPath + ".mitklabel.json";
    MITK_INFO << "Writing meta data for segmentation: " << metaFileName << std::endl;
    std::ofstream file(metaFileName);
    if (file.is_open())
    {
      file << std::setw(4) << stackContent << std::endl;
    }
    else
    {
      mitkThrow() << "Cannot write meta data. Cannot open file: " << metaFileName;
    }
  }

} // namespace
