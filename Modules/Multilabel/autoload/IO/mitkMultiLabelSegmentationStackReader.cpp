/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationStackReader.h"

#include "mitkMultilabelIOMimeTypes.h"
#include "mitkImageAccessByItk.h"
#include "mitkMultiLabelIOHelper.h"
#include "mitkLabelSetImageConverter.h"
#include <mitkLocaleSwitch.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkIPropertyPersistence.h>
#include <mitkCoreServices.h>
#include <mitkItkImageIO.h>
#include <mitkUIDManipulator.h>
#include <mitkProperties.h>

// itk
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

namespace
{

  const constexpr char* const MULTILABEL_SEGMENTATION_MODALITY_VALUE = "org.mitk.multilabel.segmentation";
  const constexpr char* const MULTILABEL_SEGMENTATION_VERSION_KEY = "org.mitk.multilabel.segmentation.version";
  const constexpr int MULTILABEL_SEGMENTATION_VERSION_VALUE = 2;
  const constexpr char* const MULTILABEL_SEGMENTATION_LABELS_INFO_KEY = "org.mitk.multilabel.segmentation.labelgroups";
  const constexpr char* const MULTILABEL_SEGMENTATION_UNLABELEDLABEL_LOCK_KEY = "org.mitk.multilabel.segmentation.unlabeledlabellock";

  mitk::Image::Pointer LoadImageBasedOnFileName(const std::string& fileName)
  {
    //We use directly itk instead of mitk::IOUtil. The latter would be more flexible, but recursive
    //calling of the io services is not working as they are statefull (e.g. the file name).
    auto imageIO = itk::ImageIOFactory::CreateImageIO(
      fileName.c_str(), itk::CommonEnums::IOFileMode::ReadMode);

    if (nullptr == imageIO)
      mitkThrow() << "Cannot load image. ITK does not support the format. Unsupported file: " << fileName;

    return mitk::ItkImageIO::LoadRawMitkImageFromImageIO(imageIO, fileName);

  }

  mitk::Image::Pointer LoadImageBasedOnFileProperty(const mitk::PropertyList* properties)
  {
    std::string fileName;
    bool imageDefined = properties->GetStringProperty("file", fileName);

    mitk::Image::Pointer image;
    if (imageDefined)
    {
      image = LoadImageBasedOnFileName(fileName);
    }

    return image;
  }

  std::pair<mitk::Label::PixelType, mitk::Label::PixelType> MakeLabelMapping(const mitk::Label* label)
  {
    mitk::Label::PixelType fileValue = label->GetValue();

    if (auto property = dynamic_cast<mitk::IntProperty*>(label->GetProperty("file_value")); nullptr != property)
    {
      fileValue = static_cast<mitk::Label::PixelType>(property->GetValue());
    }

    return std::make_pair(fileValue, label->GetValue());
  }

  mitk::LabelValueMappingVector MakeLabelsGroupMapping(const mitk::MultiLabelSegmentation::LabelVectorType& labels)
  {
    mitk::LabelValueMappingVector result;

    for (const auto& label : labels)
    {
      if (nullptr == label->GetProperty("file"))
      {
        result.push_back(MakeLabelMapping(label));
      }
    }

    return result;
  }

  mitk::MultiLabelSegmentation::ConstLabelVectorType CleanImportLabels(const mitk::MultiLabelSegmentation::LabelVectorType& labels)
  {
    mitk::MultiLabelSegmentation::ConstLabelVectorType result;

    for (const auto& label : labels)
    {
      auto cleanedLabel = label->Clone();
      cleanedLabel->RemoveProperty("file");
      cleanedLabel->RemoveProperty("file_value");
      result.push_back(cleanedLabel);
    }

    return result;
  }

  std::string FindFirstFileInJson(const nlohmann::json& j)
  {
    if (j.is_object())
    {
      for (const auto& [key, value] : j.items())
      {
        if (key == "file")
          return value;

        if (value.is_structured())  // object or array
        {
          auto result = FindFirstFileInJson(value);
          if (!result.empty())
            return result;
        }
      }
    }
    else if (j.is_array())
    {
      for (const auto& element : j)
      {
        auto result = FindFirstFileInJson(element);
        if (!result.empty())
          return result;
      }
    }

    return "";
  }

}
  mitk::MultiLabelSegmentationStackReader::MultiLabelSegmentationStackReader()
    : AbstractFileReader(MitkMultilabelIOMimeTypes::MULTILABELMETA_MIMETYPE(), "MITK Multilabel Segmentation Stack")
  {
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
  }

  mitk::IFileIO::ConfidenceLevel mitk::MultiLabelSegmentationStackReader::GetConfidenceLevel() const
  {
    if (AbstractFileReader::GetConfidenceLevel() == Unsupported)
      return Unsupported;

    std::ifstream input(this->GetLocalFileName());
    if (!input.is_open())
    {
      return Unsupported;
    }

    nlohmann::json fileContent;
    try
    {
      input >> fileContent;
    }
    catch (const nlohmann::json::parse_error& e)
    {
      MITK_ERROR << "Cannot check confidence level of reader due to parsing error. Parse error: " << e.what() << '\n';
      return Unsupported;
    }

    if (fileContent.contains("type") && fileContent["type"] == "org.mitk.multilabel.segmentation.stack")
    {
      return Supported;
    }

    return Unsupported;
  }

  std::vector<mitk::BaseData::Pointer> mitk::MultiLabelSegmentationStackReader::DoRead()
  {
    std::vector<BaseData::Pointer> result;

    std::ifstream input(this->GetLocalFileName());
    if (!input.is_open())
    {
      return result;
    }

    nlohmann::json fileContent;
    try
    {
      input >> fileContent;
    }
    catch (const nlohmann::json::parse_error& e)
    {
      MITK_ERROR << "Cannot reader data due to parsing error. Parse error: " << e.what() << '\n';
      throw;
    }

    //check version
    int version = 0;

    if (!MultiLabelIOHelper::GetValueFromJson<int>(fileContent, "version", version))
    {
      MITK_INFO << "Data has unknown version. Assuming that it can be read. Result might be invalid.";
    }

    if (version > MULTILABEL_SEGMENTATION_VERSION_VALUE)
    {
      mitkThrow() << "Data to read has unsupported version. Software is to old to ensure correct reading. Please use a compatible version of MITK or store data in another format. Version of data: " << version << "; Supported versions up to: "<<MULTILABEL_SEGMENTATION_VERSION_VALUE;
    }

    auto segmentation = MultiLabelSegmentation::New();
    bool segInitialized = false;

    //get pixel content
    auto groupInfos = MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(fileContent["groups"]);

    for (const auto& groupInfo : groupInfos)
    {
      auto groupImage = LoadImageBasedOnFileProperty(groupInfo.properties);
      auto cleanedLabels = CleanImportLabels(groupInfo.labels);

      if (!segInitialized)
      {
        //this is the first group, we need to initialize the segmentation first to ensure the
        //correct geometry.
        mitk::Image::Pointer initImage = groupImage;
        if (initImage.IsNull())
        {
          //seems to be a stack only defined with label images
          //search for the first defined file and use it for initialization.
          //not the most efficient way, but it is robust.
          auto firstFileName = FindFirstFileInJson(fileContent["groups"]);
          if (!firstFileName.empty())
          {
            initImage = LoadImageBasedOnFileName(firstFileName);
          }
        }

        if (initImage.IsNull())
        {
          mitkThrow() << "Meta seems in an invalid state. No import image was referenced to initialize the segmentation.";
        }

        segmentation->Initialize(initImage, true, false);
        segInitialized = true;
      }

      auto groupIndex = segmentation->AddGroup(cleanedLabels);

      if (groupImage.IsNotNull())
      {
        //transfer content of the labels that have no dedicated defined import image
        auto groupMapping = MakeLabelsGroupMapping(groupInfo.labels);
        TransferLabelContent(groupImage, segmentation->GetGroupImage(groupIndex),
          segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(groupIndex)),
          MultiLabelSegmentation::UNLABELED_VALUE, MultiLabelSegmentation::UNLABELED_VALUE, false, groupMapping,
          MultiLabelSegmentation::MergeStyle::Replace, MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
      }

      for (const auto& label : groupInfo.labels)
      {
        mitk::Image::Pointer labelImage = LoadImageBasedOnFileProperty(label);
        if (labelImage.IsNotNull())
        {
          if (!segInitialized)
          {
            //this is the first group, we need to initialize the segmentation first to ensure the
            //correct geometry.
            segmentation->Initialize(labelImage, false);
            segInitialized = true;
          }

          //transfer content of the labels that have no dedicated defined import image
          auto labelMapping = MakeLabelMapping(label);
          TransferLabelContent(labelImage, segmentation->GetGroupImage(groupIndex),
            segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(groupIndex)),
            MultiLabelSegmentation::UNLABELED_VALUE, MultiLabelSegmentation::UNLABELED_VALUE, false, { labelMapping },
            MultiLabelSegmentation::MergeStyle::Replace, MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
        }

      }
    }

    //meta data handling
    if (fileContent.contains("properties"))
    {
      auto loadedProperties = mitk::PropertyList::New();
      loadedProperties->FromJSON(fileContent["properties"]);

      segmentation->GetPropertyList()->ConcatenatePropertyList(loadedProperties, true);
    }

    // Handle UID
    if (fileContent.contains("uid"))
    {
      auto uid = fileContent["uid"].get<std::string>();
      if (!uid.empty())
      {
        mitk::UIDManipulator uidManipulator(segmentation);
        uidManipulator.SetUID(uid);
      }
    }

    result.push_back(segmentation.GetPointer());
    return result;
  }

  mitk::MultiLabelSegmentationStackReader* mitk::MultiLabelSegmentationStackReader::Clone() const { return new MultiLabelSegmentationStackReader(*this); }
