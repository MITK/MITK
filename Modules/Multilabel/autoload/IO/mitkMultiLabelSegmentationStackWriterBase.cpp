/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSegmentationStackWriterBase.h>
#include <mitkCustomMimeType.h>
#include <mitkImageAccessByItk.h>
#include <mitkMultiLabelIOHelper.h>
#include <mitkMultiLabelNrrdMetaData.h>
#include <mitkLabelSetImageConverter.h>
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
  constexpr int MULTILABEL_SEGMENTATION_VERSION_VALUE = 4;

  std::string JoinWithCommas(const std::vector<std::string>& items, const std::string& prefix)
  {
    std::string result;
    for (const auto& item : items)
      result += (result.empty() ? "" : ", ") + prefix + item;
    return result;
  }

  std::string DescribeExtensions(const mitk::CustomMimeType& mimeType)
  {
    return JoinWithCommas(mimeType.GetExtensions(), ".");
  }
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

    if (0 == input->GetNumberOfGroups())
    {
      mitkThrow() << "Cannot write the segmentation: it has no groups.";
    }

    const auto groupSaveStrategy = this->GetChoiceOption(OPTION_SAVE_STRATEGY) == OPTION_SAVE_STRATEGY_GROUP;
    const auto binaryValueMapping = this->GetChoiceOption(OPTION_INSTANCE_VALUE) == OPTION_INSTANCE_VALUE_BINARY;

    LocalFile localFile(this);
    const auto path = localFile.GetFileName();

    // The stem ends before the extension the mime type matched, which can contain dots
    // itself (.mitklabel.json, .nii.gz), while the stem may contain dots too.
    const auto fileStem = this->GetMimeType()->GetFilenameWithoutExtension(path);
    if (fileStem.empty())
    {
      mitkThrow() << "Cannot write the segmentation stack to \"" << path << "\": the file name has none of the extensions "
                  << DescribeExtensions(*(this->GetMimeType())) << ".";
    }

    const auto imageFileExt = this->GetStackImageExtension();
    // A path without a directory is relative to the working directory; joined with the empty
    // directory, it would point to the root of the drive.
    const auto directory = itksys::SystemTools::GetFilenamePath(path);
    const auto absFileStemPath = directory.empty() ? fileStem : itksys::SystemTools::JoinPath({ "", directory, fileStem });
    const auto relFileStemPath = itksys::SystemTools::JoinPath({"", ".", fileStem });

    auto imageIO = this->GetITKIO();
    // use compression if available
    imageIO->UseCompressionOn();
    mitk::LocaleSwitch localeSwitch("C");

    const auto writeImage = [&imageIO](const Image* image, const std::string& fileName)
      {
        ItkImageIO::PreparImageIOToWriteImage(imageIO, image);
        imageIO->SetFileName(fileName);

        ImageReadAccessor imageAccess(image);
        imageIO->Write(imageAccess.GetData());
      };

    nlohmann::json metaInfo;

    if (groupSaveStrategy)
    {
      auto groupFileNameCallback = [relFileStemPath, imageFileExt](const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::GroupIndexType groupIndex)
        {
          return GenerateGroupFilePath(relFileStemPath, groupIndex, imageFileExt);
        };

      metaInfo = MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(input, groupFileNameCallback);

      for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < input->GetNumberOfGroups(); ++groupID)
      {
        writeImage(input->GetGroupImage(groupID), GenerateGroupFilePath(absFileStemPath, groupID, imageFileExt));
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

      for (const auto labelValue : input->GetAllLabelValues())
      {
        writeImage(CreateLabelMask(input, labelValue, binaryValueMapping),
                   GenerateLabelFilePath(absFileStemPath, labelValue, imageFileExt));
      }

      // A group without labels has no label image. Written as a group image, it still carries
      // the geometry, which the reader needs when no group has labels.
      for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < input->GetNumberOfGroups(); ++groupID)
      {
        if (input->GetLabelValuesByGroup(groupID).empty())
        {
          metaInfo[groupID]["_file"] = GenerateGroupFilePath(relFileStemPath, groupID, imageFileExt);
          writeImage(input->GetGroupImage(groupID), GenerateGroupFilePath(absFileStemPath, groupID, imageFileExt));
        }
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
      RemoveReservedProperties(cleanedProps, GetMultiLabelReservedKeys());
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
