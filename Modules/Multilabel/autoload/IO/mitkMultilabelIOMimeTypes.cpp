/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultilabelIOMimeTypes.h"
#include <mitkIOMimeTypes.h>
#include <mitkLogMacros.h>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include <itkNrrdImageIO.h>
#include "itkMetaDataObject.h"

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType::MitkSegmentationTaskListMimeType()
  : CustomMimeType(SEGMENTATIONTASKLIST_MIMETYPE_NAME())
{
  this->AddExtension("json");
  this->SetCategory("MITK Segmentation Task List");
  this->SetComment("MITK Segmentation Task List");
}

bool mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType::AppliesTo(const std::string& path) const
{
  bool result = CustomMimeType::AppliesTo(path);

  if (!std::filesystem::exists(path)) // T18572
    return result;

  std::ifstream file(path);

  if (!file.is_open())
    return false;

  auto json = nlohmann::json::parse(file, nullptr, false);

  if (json.is_discarded() || !json.is_object())
    return false;

  if ("MITK Segmentation Task List" != json.value("FileFormat", ""))
    return false;

  if (1 != json.value<int>("Version", 0))
    return false;

  return true;
}

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType* mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType::Clone() const
{
  return new MitkSegmentationTaskListMimeType(*this);
}

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType mitk::MitkMultilabelIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE()
{
  return MitkSegmentationTaskListMimeType();
}

std::string mitk::MitkMultilabelIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".segmentationtasklist";
}

mitk::MitkMultilabelIOMimeTypes::LegacyLabelSetMimeType::LegacyLabelSetMimeType()
  : CustomMimeType(LEGACYLABELSET_MIMETYPE_NAME())
{
  this->AddExtension("nrrd");
  this->SetCategory("MITK LabelSetImage");
  this->SetComment("MITK LabelSetImage (legacy format)");
}

bool mitk::MitkMultilabelIOMimeTypes::LegacyLabelSetMimeType::AppliesTo(const std::string& path) const
{
  bool canRead = CustomMimeType::AppliesTo(path);

  if (!std::filesystem::exists(path)) // T18572
    return canRead;

  if (!canRead)
  {
    return false;
  }

  std::string value("");
  try
  {
    std::ifstream file(path);

    if (!file.is_open())
      return false;

    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(path);
    io->ReadImageInformation();

    itk::MetaDataDictionary imgMetaDataDictionary = io->GetMetaDataDictionary();
    itk::ExposeMetaData<std::string>(imgMetaDataDictionary, "modality", value);
  }
  catch(const std::exception& e)
  {
    MITK_DEBUG << "Error while try to anylize NRRD file for LegacyLabelSetMimeType. File: " << path <<"; Error: " << e.what();
  }
  catch(...)
  {
    MITK_DEBUG << "Unkown error while try to anylize NRRD file for LegacyLabelSetMimeType. File: " << path;
  }

  if (value.compare("org.mitk.image.multilabel") != 0)
  {
    return false;
  }

  return true;
}

mitk::MitkMultilabelIOMimeTypes::LegacyLabelSetMimeType* mitk::MitkMultilabelIOMimeTypes::LegacyLabelSetMimeType::Clone() const
{
  return new LegacyLabelSetMimeType(*this);
}

mitk::MitkMultilabelIOMimeTypes::LegacyLabelSetMimeType mitk::MitkMultilabelIOMimeTypes::LEGACYLABELSET_MIMETYPE()
{
  return LegacyLabelSetMimeType();
}

std::string mitk::MitkMultilabelIOMimeTypes::LEGACYLABELSET_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".legacylabelsetimage";
}

std::vector<mitk::CustomMimeType*> mitk::MitkMultilabelIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;
  mimeTypes.push_back(SEGMENTATIONTASKLIST_MIMETYPE().Clone());
  mimeTypes.push_back(LEGACYLABELSET_MIMETYPE().Clone());
  return mimeTypes;
}
