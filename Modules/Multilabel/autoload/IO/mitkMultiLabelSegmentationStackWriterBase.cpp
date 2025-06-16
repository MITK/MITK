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
  const constexpr char* const OPTION_SAVE_STRATEGY = "Save strategy";
  const constexpr char* const OPTION_SAVE_STRATEGY_INSTANCE = "instance";
  const constexpr char* const OPTION_SAVE_STRATEGY_GROUP = "group";
  const constexpr char* const OPTION_INSTANCE_VALUE = "Instance value";
  const constexpr char* const OPTION_INSTANCE_VALUE_BINARY = "binary";
  const constexpr char* const OPTION_INSTANCE_VALUE_ORIGINAL = "original";
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
      return Supported;
    else
      return Unsupported;
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

    const auto fileExt = itksys::SystemTools::GetFilenameExtension(path);
    const auto fileStemPath = itksys::SystemTools::JoinPath({"", itksys::SystemTools::GetFilenamePath(path), itksys::SystemTools::GetFilenameWithoutExtension(path)});

    const auto saveStrategy = options.find(OPTION_SAVE_STRATEGY)->second.ToString();

    nlohmann::json metaInfo;

    if (groupSaveStrategy)
    {
      auto groupFileNameCallback = [fileStemPath, fileExt](const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::GroupIndexType groupIndex)
        {
          return fileStemPath + "_group_" + std::to_string(groupIndex) + "" + fileExt;
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
        imageIO->SetFileName(groupFileNameCallback(input, groupID));

        ImageReadAccessor imageAccess(groupImage);
        imageIO->Write(imageAccess.GetData());
      }
    }
    else
    {
      auto labelFileNameCallback = [fileStemPath, fileExt](const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::LabelValueType labelValue)
        {
          return fileStemPath + "_label_" + std::to_string(labelValue) + "" + fileExt;
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
        imageIO->SetFileName(labelFileNameCallback(input, labelValue));

        ImageReadAccessor imageAccess(labelImage);
        imageIO->Write(imageAccess.GetData());
      }
    }

    nlohmann::json stackContent;
    stackContent["version"] = "1";
    stackContent["type"] = "org.mitk.multilabel.segmentation.stack";
    stackContent["groups"] = metaInfo;
    if (auto properties = input->GetPropertyList(); !properties->IsEmpty())
      properties->ToJSON(stackContent["properties"]);
    stackContent["uid"] = input->GetUID();

    const auto metaFileName = fileStemPath + ".mitklabel.json";
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
